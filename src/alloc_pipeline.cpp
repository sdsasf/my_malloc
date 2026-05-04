// AllocPipeline: chain-of-responsibility allocation strategies

#include "my_ptmalloc/alloc_pipeline.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/tcache.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/types.h"
#include "my_ptmalloc/chunk.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/observer.h"
#include "my_ptmalloc/heap.h"

namespace my_ptmalloc {

// ─── Strategy 1: Tcache (lock-free) ───

bool TcacheAlloc::try_alloc(Arena& arena, size_t nb, void*& result) noexcept {
    (void)arena;  // tcache is thread-local, no arena needed
    if (!tcache) return false;

    TcacheIdx tidx = csize2tidx(ChunkSize{nb});
    if (tidx.value >= TCACHE_MAX_BINS) return false;

    void* p = tcache->alloc(tidx);
    if (p) {
        Chunk* chunk = Chunk::from_user_ptr(p);
        // Defensive: validate chunk header before mark_inuse.
        // If chunk size is 0, next_chunk() returns self and mark_inuse
        // would corrupt the chunk's own size field (0→1).
        size_t cs = chunk->chunk_size().value;
        if (cs < MINSIZE || (cs & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[TcacheAlloc] CORRUPT: tidx=%zu raw_size=0x%zx chunk=%p\n",
                    tidx.value, chunk->size, (void*)chunk);
#endif
            // Don't return corrupt chunk — fall through to other strategies
            return false;
        }
        chunk->mark_inuse();
        result = p;
        return true;
    }
    return false;
}

// ─── Strategy 2: Fastbin (CAS lock-free) ───

bool FastbinAlloc::try_alloc(Arena& arena, size_t nb, void*& result) noexcept {
    FastbinIdx fidx = fastbin_index(ChunkSize{nb});
    if (fidx.value >= NFASTBINS) return false;

    Chunk* p = arena.bins_.fast().pop(fidx);
    if (p) {
        // DEBUG
        if (p->chunk_size().value < MINSIZE || (p->chunk_size().value & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[FastbinAlloc] BAD: fidx=%zu raw_size=0x%zx chunk=%p\n",
                    fidx.value, p->size, (void*)p);
#endif
            return false;
        }
        p->mark_inuse();
        result = p->user_data();
        return true;
    }
    return false;
}

// ─── Strategy 3: Smallbin ───

bool SmallbinAlloc::try_alloc(Arena& arena, size_t nb, void*& result) noexcept {
    if (!in_smallbin_range(ChunkSize{nb})) return false;

    SmallbinIdx sidx = smallbin_index(ChunkSize{nb});
    Chunk* p = arena.bins_.small().alloc(sidx);
    if (p) {
        // DEBUG
        if (p->chunk_size().value < MINSIZE || (p->chunk_size().value & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[SmallbinAlloc] BAD: sidx=%zu raw_size=0x%zx chunk=%p\n",
                    sidx.value, p->size, (void*)p);
#endif
            return false;
        }
        p->mark_inuse();
        result = p->user_data();
        return true;
    }
    return false;
}

// ─── Strategy 4: Unsorted bin scan ───

bool UnsortedAlloc::try_alloc(Arena& arena, size_t nb, void*& result) noexcept {
    // First consolidate fastbins if needed
    if (arena.has_fastchunks()) {
        malloc_consolidate(arena);
    }

    // Scan unsorted bin
    Chunk* found = arena.bins_.unsorted().scan_and_sort(
        ChunkSize{nb}, arena.bins_.small(), arena.bins_.large(),
        tcache, arena.last_remainder_);
    if (found) {
        // DEBUG
        if (found->chunk_size().value < MINSIZE || (found->chunk_size().value & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[UnsortedAlloc] BAD: raw_size=0x%zx chunk=%p\n",
                    found->size, (void*)found);
#endif
            return false;
        }
        found->mark_inuse();
        result = found->user_data();
        return true;
    }
    return false;
}

// ─── Strategy 5: Large bin best-fit ───

bool LargebinAlloc::try_alloc(Arena& arena, size_t nb, void*& result) noexcept {
    auto [victim, remainder] = arena.bins_.large().alloc_split(
        ChunkSize{nb}, arena.bins_.unsorted());

    if (victim) {
        // DEBUG
        if (victim->chunk_size().value < MINSIZE || (victim->chunk_size().value & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[LargebinAlloc] BAD: raw_size=0x%zx chunk=%p\n",
                    victim->size, (void*)victim);
#endif
            return false;
        }
        victim->mark_inuse();
        if (remainder) arena.set_last_remainder(remainder);
        result = victim->user_data();
        return true;
    }
    return false;
}

// ─── Strategy 6: Top chunk ───

bool TopChunkAlloc::try_alloc(Arena& arena, size_t nb, void*& result) noexcept {
    Chunk* top = arena.top();
    if (!top || top->chunk_size().value < nb) return false;

    size_t top_size = top->chunk_size().value;
    size_t remainder_size = top_size - nb;
    bool non_main = g_arena_manager && (&arena != g_arena_manager->get_main_arena());

    if (remainder_size >= MINSIZE) {
        // Split top chunk
        Chunk* new_top = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(top) + nb);
        new_top->prev_size = nb;
        new_top->set_head(ChunkSize{remainder_size}, ChunkFlag::PREV_INUSE);
        arena.set_top(new_top);
        top->set_head(ChunkSize{nb}, ChunkFlag::PREV_INUSE);
    } else {
        // Use entire top chunk
        arena.set_top(nullptr);
        top->set_head(ChunkSize{top_size}, ChunkFlag::PREV_INUSE);
    }

    if (non_main) top->size |= NON_MAIN_ARENA_BIT;

    // DEBUG
    if (top->chunk_size().value < MINSIZE || (top->chunk_size().value & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
        fprintf(stderr, "[TopChunk] BAD: nb=%zu top_size=%zu remainder=%zu non_main=%d raw_size=0x%zx chunk=%p\n",
                nb, top_size, remainder_size, non_main, top->size, (void*)top);
#endif
    }

    result = top->user_data();
    return true;
}

// ─── Strategy 7: System allocation (mmap/new heap) ───

bool SysAlloc::try_alloc(Arena& arena, size_t nb, void*& result) noexcept {
    ThresholdPolicy* thresh = g_arena_manager ? g_arena_manager->threshold() : nullptr;
    size_t mmap_thresh = thresh ? thresh->mmap_threshold() : DEFAULT_MMAP_THRESHOLD;
    bool non_main = g_arena_manager && (&arena != g_arena_manager->get_main_arena());

    if (nb >= mmap_thresh) {
        // Large allocation: direct mmap (always IS_MMAPPED, never in arena bins)
        SysMemory* mem = g_arena_manager ? g_arena_manager->sys_memory() : nullptr;
        if (!mem) return false;

        size_t alloc_size = nb + CHUNK_HDR_SZ;
        alloc_size = (alloc_size + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;

        void* p = mem->map(alloc_size, MALLOC_ALIGNMENT);
        if (!p) return false;

        Chunk* chunk = reinterpret_cast<Chunk*>(p);
        chunk->prev_size = 0;
        chunk->set_head(ChunkSize{alloc_size},
            ChunkFlag::PREV_INUSE | ChunkFlag::IS_MMAPPED);

        result = chunk->user_data();
        return true;
    }

    // Extend heap for non-large allocations
    SysMemory* mem = g_arena_manager ? g_arena_manager->sys_memory() : nullptr;
    if (!mem) return false;

    size_t heap_size = std::max(nb + 2 * MINSIZE, (size_t)HEAP_MAX_SIZE);
    heap_size = (heap_size + HEAP_MAX_SIZE - 1) & ~(HEAP_MAX_SIZE - 1);

    void* heap = nullptr;
    size_t usable_size = heap_size - MINSIZE;
    if (non_main) {
        HeapInfo* h = HeapInfo::new_heap(heap_size, &arena, nullptr);
        if (!h) return false;
        uintptr_t addr = reinterpret_cast<uintptr_t>(h) + sizeof(HeapInfo);
        addr = (addr + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;
        heap = reinterpret_cast<void*>(addr);
        size_t prefix = addr - reinterpret_cast<uintptr_t>(h);
        if (heap_size <= prefix + MINSIZE) return false;
        usable_size = heap_size - prefix - MINSIZE;
    } else {
        heap = mem->map(heap_size, MALLOC_ALIGNMENT);
        if (!heap) return false;
    }

    Chunk* chunk = reinterpret_cast<Chunk*>(heap);
    chunk->prev_size = 0;

    Chunk* fencepost = reinterpret_cast<Chunk*>(
        reinterpret_cast<uintptr_t>(heap) + usable_size);
    fencepost->prev_size = usable_size;
    fencepost->set_head(ChunkSize{MINSIZE}, ChunkFlag::PREV_INUSE);
    if (non_main) fencepost->size |= NON_MAIN_ARENA_BIT;

    size_t remainder = usable_size - nb;
    if (remainder >= MINSIZE) {
        chunk->set_head(ChunkSize{nb}, ChunkFlag::PREV_INUSE);
        if (non_main) chunk->size |= NON_MAIN_ARENA_BIT;

        Chunk* new_top = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(heap) + nb);
        new_top->prev_size = 0;
        new_top->set_head(ChunkSize{remainder}, ChunkFlag::PREV_INUSE);
        if (non_main) new_top->size |= NON_MAIN_ARENA_BIT;

        if (arena.top()) {
            arena.bins_.unsorted().push(arena.top());
        }
        arena.set_top(new_top);
    } else {
        chunk->set_head(ChunkSize{usable_size}, ChunkFlag::PREV_INUSE);
        if (non_main) chunk->size |= NON_MAIN_ARENA_BIT;
    }

    arena.update_system_mem(heap_size);

    // DEBUG
    if (chunk->chunk_size().value < MINSIZE || (chunk->chunk_size().value & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
        fprintf(stderr, "[SysAlloc] BAD: nb=%zu heap_size=%zu remainder=%zu non_main=%d raw_size=0x%zx chunk=%p\n",
                nb, heap_size, remainder, non_main, chunk->size, (void*)chunk);
#endif
    }

    result = chunk->user_data();
    return true;
}

// ─── Pipeline ───

AllocPipeline::AllocPipeline() noexcept {
    strategies_ = {
        &tcache_,
        &fastbin_,
        &smallbin_,
        &unsorted_,
        &largebin_,
        &topchunk_,
        &sys_,
    };
}

bool AllocPipeline::execute(Arena& arena, size_t nb, void*& result) noexcept {
    for (auto* strategy : strategies_) {
        if (strategy->try_alloc(arena, nb, result)) {
            return true;
        }
    }
    return false;
}

} // namespace my_ptmalloc
