// Core free implementation

#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/thread_registry.h"
#include "my_ptmalloc/tcache.h"
#include "my_ptmalloc/observer.h"
#include "my_ptmalloc/slab_allocator.h"
#include "my_ptmalloc/family_allocators.h"
#include "my_ptmalloc/runtime_allocator.h"
#include "my_ptmalloc/allocator_lab.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/types.h"
#include "my_ptmalloc/chunk.h"
#include "my_ptmalloc/heap.h"

namespace my_ptmalloc {

static bool chunk_is_free(Arena& arena, Chunk* p) noexcept {
    if (!p || p == arena.top()) return false;
    return arena.bins_.contains_free_chunk(p);
}

static void consolidate_and_free(Arena& arena, Chunk* p) noexcept {
    size_t psize = p->chunk_size().value;
    ChunkFlag orig_flags = p->flags();  // Preserve NON_MAIN_ARENA and other flags

    // Forward merge: if next chunk is free
    Chunk* next = p->next_chunk();
    if (chunk_is_free(arena, next)) {
        size_t next_size = next->chunk_size().value;
        if (arena.bins_.unlink_free_chunk(next)) {
            if (arena.last_remainder_ == next) arena.last_remainder_ = nullptr;
            psize += next_size;
        } else {
            // Inconsistent metadata: clear prev_inuse so future merges can repair
            p->clear_previnuse();
        }
    }

    // Backward merge: if previous chunk is free
    if (!p->prev_inuse()) {
        size_t prev_sz = p->prev_size;
        uintptr_t p_addr = reinterpret_cast<uintptr_t>(p);
        if (prev_sz >= MINSIZE && (prev_sz & MALLOC_ALIGN_MASK) == 0 &&
            prev_sz <= p_addr) {
            Chunk* prev = reinterpret_cast<Chunk*>(p_addr - prev_sz);
            size_t prev_size = prev->chunk_size().value;
            if (prev_size == prev_sz && arena.bins_.unlink_free_chunk(prev)) {
                if (arena.last_remainder_ == prev) arena.last_remainder_ = nullptr;
                psize += prev_size;
                p = prev;
                orig_flags = p->flags();
            }
        }
    }

    // Check if next chunk is top - merge into top
    Chunk* new_next = reinterpret_cast<Chunk*>(
        reinterpret_cast<uintptr_t>(p) + psize);
    if (arena.top() == new_next) {
        psize += arena.top()->chunk_size().value;
        p->set_head(ChunkSize{psize}, orig_flags | ChunkFlag::PREV_INUSE);
        arena.set_top(p);
        systrim(arena, DEFAULT_TOP_PAD);
        return;
    }

    // Defensive: verify merged chunk is valid before writing headers
    if (psize < MINSIZE || (psize & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
        fprintf(stderr, "[consolidate] BAD merged size: p=%p psize=0x%zx\n",
                (void*)p, psize);
#endif
        return;  // don't push corrupt chunk to bin
    }

    // Set merged chunk header (preserve arena ownership flag)
    p->set_head(ChunkSize{psize}, orig_flags | ChunkFlag::PREV_INUSE);
    p->set_foot(ChunkSize{psize});

    // Clear PREV_INUSE on next chunk (avoid self if corruption produced zero)
    if (new_next != p) {
        new_next->size &= ~PREV_INUSE_BIT;
    }

    // Place into unsorted bin
    arena.bins_.unsorted().push(p);
}

void my_free(void* ptr) noexcept {
    if (!ptr) return;

    if (runtime_mode_uses_family_allocators() && family_free(ptr)) {
        if (allocator_stats_enabled_fast()) stats_record_free(AllocPath::Slab);
        if (allocator_trace_enabled_fast()) trace_record(AllocOp::Free, AllocPath::Slab, 0, ptr);
        return;
    }

    if (slab_free(ptr)) {
        if (allocator_stats_enabled_fast()) stats_record_free(AllocPath::Slab);
        if (allocator_trace_enabled_fast()) trace_record(AllocOp::Free, AllocPath::Slab, 0, ptr);
        return;
    }

    Chunk* p = Chunk::from_user_ptr(ptr);
    size_t chunk_size = p->chunk_size().value;

    // DEBUG: validate chunk header
    if (chunk_size < MINSIZE || (chunk_size & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
        fprintf(stderr, "[free] CORRUPT HEADER: ptr=%p chunk=%p raw_size=0x%zx cs=%zu\n",
                ptr, (void*)p, p->size, chunk_size);
#endif
        return;
    }

    // If mmap'd chunk, unmap directly
    if (p->is_mmapped()) {
        if (g_arena_manager) {
            SysMemory* mem = g_arena_manager->sys_memory();
            if (mem) mem->unmap(p, chunk_size);
        }
        if (allocator_stats_enabled_fast()) stats_record_free(AllocPath::Mmap);
        if (allocator_trace_enabled_fast()) trace_record(AllocOp::Free, AllocPath::Mmap, chunk_size, ptr);
        return;
    }

    // Try tcache fast path (lock-free)
    if (tcache && in_tcache_range(ChunkSize{chunk_size})) {
        TcacheIdx tidx = csize2tidx(ChunkSize{chunk_size});
        // Double-free detection: check if this chunk is already in tcache
        TcacheEntry* e = reinterpret_cast<TcacheEntry*>(p->user_data());
        if (tcache->is_double_free(e)) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[free] DOUBLE FREE DETECTED: ptr=%p chunk=%p tidx=%zu\n",
                    ptr, (void*)p, tidx.value);
#endif
            return;  // refuse to free
        }
        if (tcache->free(tidx, p)) {
            if (allocator_stats_enabled_fast()) stats_record_free(AllocPath::Tcache);
            if (allocator_trace_enabled_fast()) trace_record(AllocOp::Free, AllocPath::Tcache, chunk_size, ptr);
            return;
        }
    }

    // Find the arena that owns this chunk
    Arena* target_arena = HeapInfo::arena_for_chunk(p, p->is_main_arena());
    if (!target_arena) return;

    target_arena->lock();
    consolidate_and_free(*target_arena, p);
    target_arena->unlock();
    if (allocator_stats_enabled_fast()) stats_record_free(AllocPath::Arena);
    if (allocator_trace_enabled_fast()) trace_record(AllocOp::Free, AllocPath::Arena, chunk_size, ptr);
}

} // namespace my_ptmalloc
