// malloc_consolidate and systrim implementations

#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/chunk.h"
#include "my_ptmalloc/observer.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/threshold.h"
#include "my_ptmalloc/sys_memory.h"
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstdint>

namespace my_ptmalloc {

static bool chunk_is_free(Arena& arena, Chunk* p) noexcept {
    if (!p || p == arena.top()) return false;
    return arena.bins_.contains_free_chunk(p);
}

void malloc_consolidate(Arena& arena) noexcept {
    if (!arena.has_fastchunks()) return;

    // Drain all fastbins, merging with adjacent free chunks
    arena.bins_.fast().drain([&arena](Chunk* p) {
        size_t psize = p->chunk_size().value;

        // Defensive: validate chunk size before processing
        if (psize < MINSIZE || (psize & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[consolidate] CORRUPT fastbin chunk: %p raw=0x%zx\n",
                    (void*)p, p->size);
#endif
            return;  // skip this chunk
        }

        // Forward merge: if next chunk is free and in a bin
        Chunk* next = p->next_chunk();
        if (chunk_is_free(arena, next)) {
            size_t next_size = next->chunk_size().value;
            if (arena.bins_.unlink_free_chunk(next)) {
                if (arena.last_remainder_ == next) arena.last_remainder_ = nullptr;
                psize += next_size;
            } else {
                p->clear_previnuse();
            }
        }

        // Backward merge: if previous chunk is free and in a bin
        if (!p->prev_inuse()) {
            Chunk* prev = p->prev_chunk();
            size_t prev_size = prev->chunk_size().value;
            if (prev_size >= MINSIZE && (prev_size & MALLOC_ALIGN_MASK) == 0 &&
                arena.bins_.unlink_free_chunk(prev)) {
                if (arena.last_remainder_ == prev) arena.last_remainder_ = nullptr;
                psize += prev_size;
                p = prev;
            }
        }

        // Defensive: validate merged size before writing headers
        if (psize < MINSIZE || (psize & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[consolidate] BAD merged size: p=%p psize=0x%zx\n",
                    (void*)p, psize);
#endif
            return;
        }

        p->set_head(ChunkSize{psize}, ChunkFlag::PREV_INUSE);
        p->set_foot(ChunkSize{psize});
        arena.bins_.unsorted().push(p);
    });

    arena.clear_fastchunks();
    g_observer->on_consolidate(0);
}

bool systrim(Arena& arena, size_t pad) noexcept {
    Chunk* top = arena.top();
    if (!top) return false;

    size_t top_size = top->chunk_size().value;

    // Compute how much to release: everything above (threshold + pad)
    ThresholdPolicy* thresh = g_arena_manager ? g_arena_manager->threshold() : nullptr;
    size_t trim_thresh = thresh ? thresh->trim_threshold() : DEFAULT_TRIM_THRESHOLD;

    if (top_size < trim_thresh + pad + 2 * MINSIZE) return false;

    size_t release = top_size - pad - MINSIZE;
    // Align release down to page size
    long pagesize = sysconf(_SC_PAGESIZE);
    if (pagesize <= 0) pagesize = 4096;
    release = release & ~(static_cast<size_t>(pagesize) - 1);
    if (release < static_cast<size_t>(pagesize)) return false;

    // Compute the address range to release
    uintptr_t top_addr = reinterpret_cast<uintptr_t>(top);
    uintptr_t release_start = (top_addr + top_size - release + pagesize - 1)
                              & ~(static_cast<uintptr_t>(pagesize) - 1);
    size_t release_len = (top_addr + top_size) - release_start;
    if (release_len < static_cast<size_t>(pagesize)) return false;

    // Use madvise to release physical pages (keeps virtual mapping)
    if (madvise(reinterpret_cast<void*>(release_start), release_len,
                MADV_DONTNEED) != 0) {
        return false;
    }

    // Shrink top chunk
    size_t new_top_size = top_size - release_len;
    top->set_head(ChunkSize{new_top_size}, ChunkFlag::PREV_INUSE);

    // Reduce system_mem (shrink, not grow)
    if (arena.system_mem_ >= release_len) {
        arena.system_mem_ -= release_len;
    } else {
        arena.system_mem_ = 0;
    }
    g_observer->on_trim(release_len);
    return true;
}

} // namespace my_ptmalloc
