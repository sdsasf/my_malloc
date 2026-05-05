// Core malloc implementation

#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/alloc_pipeline.h"
#include "my_ptmalloc/thread_registry.h"
#include "my_ptmalloc/tcache.h"
#include "my_ptmalloc/observer.h"
#include "my_ptmalloc/slab_allocator.h"
#include "my_ptmalloc/family_allocators.h"
#include "my_ptmalloc/runtime_allocator.h"
#include "my_ptmalloc/allocator_lab.h"
#include "my_ptmalloc/adaptive_allocator.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/types.h"
#include "my_ptmalloc/chunk.h"

namespace my_ptmalloc {

void* my_malloc(size_t size) noexcept {
    // Initialize on first use
    if (!g_arena_manager) my_malloc_init();

    // Edge case: zero-size allocation
    if (size == 0) size = 1;

    // Independent adaptive allocator — bypass all teaching backends
    if (runtime_mode_is_adaptive()) {
        return adaptive_malloc(size);
    }

    RuntimeAllocatorKind impl = runtime_select_allocator(size);
    switch (impl) {
        case RuntimeAllocatorKind::TcmallocLike:
            return tcmalloc_like_malloc(size);
        case RuntimeAllocatorKind::JemallocLike:
            return jemalloc_like_malloc(size);
        case RuntimeAllocatorKind::MimallocLike:
            return mimalloc_like_malloc(size);
        case RuntimeAllocatorKind::Hybrid:
        case RuntimeAllocatorKind::Ptmalloc:
            break;
    }

    if (impl == RuntimeAllocatorKind::Hybrid) {
        if (void* slab = slab_malloc(size)) {
            if (allocator_stats_enabled_fast()) stats_record_alloc(AllocPath::Slab);
            if (allocator_trace_enabled_fast()) trace_record(AllocOp::Malloc, AllocPath::Slab, size, slab);
            return slab;
        }
    }

    // Compute aligned chunk size
    ChunkSize nb = request2size(UserSize{size});

    // Ensure tcache is initialized
    if (tcache == nullptr) {
        tcache_init();
    }

    TcacheIdx tidx = csize2tidx(nb);
    if (tidx.value < TCACHE_MAX_BINS) {
        void* cached = tcache->alloc(tidx);
        if (cached) {
            Chunk* c = Chunk::from_user_ptr(cached);
            if (c->chunk_size().value >= MINSIZE &&
                (c->chunk_size().value & MALLOC_ALIGN_MASK) == 0) {
                c->mark_inuse();
                if (allocator_stats_enabled_fast()) stats_record_alloc(AllocPath::Tcache);
                if (allocator_trace_enabled_fast()) trace_record(AllocOp::Malloc, AllocPath::Tcache, size, cached);
                return cached;
            }
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[malloc] corrupt tcache chunk: result=%p chunk=%p size=0x%zx\n",
                    cached, (void*)c, c->size);
#endif
        }
    }

    // Ensure thread is registered and has an arena
    if (thread_arena == nullptr) {
        ThreadRegistry::register_thread();
    }

    // Get arena (locks it)
    Arena* av = g_arena_manager->get_arena(nb.value);

    // Execute allocation pipeline (tcache is first strategy, lock-free inside)
    void* result = nullptr;
    bool success = g_alloc_pipeline->execute(*av, nb.value, result);

    // Unlock arena
    av->unlock();

    if (success && result) {
        // DEBUG: verify chunk header after allocation
        Chunk* c = Chunk::from_user_ptr(result);
        if (c->chunk_size().value < MINSIZE || (c->chunk_size().value & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[malloc] CORRUPT AFTER ALLOC: result=%p chunk=%p size=0x%zx\n",
                    result, (void*)c, c->size);
#endif
        }
    }

    if (success && result) {
        if (allocator_stats_enabled_fast()) stats_record_alloc(AllocPath::Arena);
        if (allocator_trace_enabled_fast()) trace_record(AllocOp::Malloc, AllocPath::Arena, size, result);
    } else {
        if (allocator_trace_enabled_fast()) trace_record(AllocOp::Malloc, AllocPath::None, size, nullptr);
    }
    return success ? result : nullptr;
}

} // namespace my_ptmalloc
