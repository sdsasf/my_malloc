// Coalescing implementation: consolidate, merge, systrim.

#include "coalesce.h"
#include "arena_manager.h"
#include "sys_memory.h"
#include "tcache.h"

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Helpers ───

static bool chunk_is_free(Arena& arena, Chunk* p) noexcept {
    if (!p || p == arena.top()) return false;
    return arena.bins_.contains_free_chunk(p);
}

// ─── malloc_consolidate ───
void malloc_consolidate(Arena& arena) noexcept {
    PTMALLOC_LOG("malloc_consolidate");

    for (unsigned i = 0; i < NFASTBINS; ++i) {
        Chunk* p = arena.bins_.fast(FastbinIdx{i}).pop_all();
        if (!p) continue;

        // Process all chunks in this fastbin
        while (p) {
            Chunk* next = p->fd;
            size_t psize = p->chunk_size();

            // Check if adjacent chunks are free and merge them
            Chunk* next_chunk = p->next_chunk();

            // Forward merge: if next chunk is free (not top, not inuse)
            if (next_chunk != arena.top() && !next_chunk->prev_inuse()) {
                size_t next_size = next_chunk->chunk_size();
                if (arena.bins_.unlink_free_chunk(next_chunk)) {
                    if (arena.last_remainder_ == next_chunk)
                        arena.set_last_remainder(nullptr);
                    psize += next_size;
                } else {
                    // Metadata inconsistency — repair by clearing prev_inuse
                    p->clear_previnuse();
                }
            }

            // Backward merge: if previous chunk is marked free
            if (!p->prev_inuse()) {
                size_t prev_sz = p->prev_size;
                uintptr_t p_addr = reinterpret_cast<uintptr_t>(p);
                if (prev_sz >= MINSIZE && (prev_sz & MALLOC_ALIGN_MASK) == 0
                    && prev_sz <= p_addr) {
                    Chunk* prev = reinterpret_cast<Chunk*>(p_addr - prev_sz);
                    if (prev->chunk_size() == prev_sz
                        && arena.bins_.unlink_free_chunk(prev)) {
                        if (arena.last_remainder_ == prev)
                            arena.set_last_remainder(nullptr);
                        psize += prev_sz;
                        p = prev;
                    }
                }
            }

            // Check if merged chunk is adjacent to top — merge into top
            Chunk* new_next = reinterpret_cast<Chunk*>(
                reinterpret_cast<uintptr_t>(p) + psize);
            if (arena.top() == new_next) {
                psize += arena.top()->chunk_size();
                p->set_head(ChunkSize{psize}, ChunkFlag::PREV_INUSE);
                arena.set_top(p);
            } else {
                // Place merged chunk in unsorted bin
                p->set_head(ChunkSize{psize}, ChunkFlag::PREV_INUSE);
                p->set_foot(ChunkSize{psize});

                // Clear PREV_INUSE on next chunk
                if (new_next != p) {
                    new_next->size &= ~PREV_INUSE_BIT;
                }

                arena.bins_.unsorted().push_front(p);
                arena.bins_.mark_bin(UNSORTED_BIN_IDX);
            }

            p = next;
        }
    }
}

// ─── consolidate_and_free ───
void consolidate_and_free(Arena& arena, Chunk* p) noexcept {
    size_t psize = p->chunk_size();
    bool non_main = !p->is_main_arena();

    // Forward merge
    Chunk* next = p->next_chunk();
    if (chunk_is_free(arena, next)) {
        size_t next_size = next->chunk_size();
        if (arena.bins_.unlink_free_chunk(next)) {
            if (arena.last_remainder_ == next)
                arena.set_last_remainder(nullptr);
            psize += next_size;
        } else {
            p->clear_previnuse();
        }
    }

    // Backward merge
    if (!p->prev_inuse()) {
        size_t prev_sz = p->prev_size;
        uintptr_t p_addr = reinterpret_cast<uintptr_t>(p);
        if (prev_sz >= MINSIZE && (prev_sz & MALLOC_ALIGN_MASK) == 0
            && prev_sz <= p_addr) {
            Chunk* prev = reinterpret_cast<Chunk*>(p_addr - prev_sz);
            size_t prev_size = prev->chunk_size();
            if (prev_size == prev_sz && arena.bins_.unlink_free_chunk(prev)) {
                if (arena.last_remainder_ == prev)
                    arena.set_last_remainder(nullptr);
                psize += prev_size;
                p = prev;
            }
        }
    }

    // Check if next is top — merge into top
    Chunk* new_next = reinterpret_cast<Chunk*>(
        reinterpret_cast<uintptr_t>(p) + psize);
    if (arena.top() == new_next) {
        psize += arena.top()->chunk_size();
        ChunkFlag flags = ChunkFlag::PREV_INUSE;
        if (non_main) flags = flags | ChunkFlag::NON_MAIN_ARENA;
        p->set_head(ChunkSize{psize}, flags);
        arena.set_top(p);
        systrim(arena, DEFAULT_TOP_PAD);
        return;
    }

    // Validate merged size before writing headers
    if (psize < MINSIZE || (psize & MALLOC_ALIGN_MASK) != 0) {
        PTMALLOC_LOG("consolidate: BAD merged size %zu", psize);
        return;
    }

    // Set merged chunk header
    ChunkFlag flags = ChunkFlag::PREV_INUSE;
    if (non_main) flags = flags | ChunkFlag::NON_MAIN_ARENA;
    p->set_head(ChunkSize{psize}, flags);
    p->set_foot(ChunkSize{psize});

    // Clear PREV_INUSE on next chunk (skip if corruption produced self-reference)
    if (new_next != p) {
        new_next->size &= ~PREV_INUSE_BIT;
    }

    // Place into unsorted bin
    arena.bins_.unsorted().push_front(p);
    arena.bins_.mark_bin(UNSORTED_BIN_IDX);
}

// ─── systrim ───
bool systrim(Arena& arena, size_t pad) noexcept {
    Chunk* top = arena.top();
    if (!top) return false;

    size_t top_size = top->chunk_size();

    // Only trim if top is large enough
    if (top_size <= DEFAULT_TRIM_THRESHOLD + pad) return false;

    // Release excess memory from the top chunk
    SysMemory* sysmem = g_arena_manager->sys_memory();
    void* top_start = static_cast<void*>(top);

    size_t keep = pad + DEFAULT_TRIM_THRESHOLD;
    if (keep > top_size) keep = top_size;

    bool result = sysmem->trim(top_start, top_size, keep, 0);
    if (result) {
        // Adjust top chunk size
        size_t new_top_size = keep;
        if (new_top_size < MINSIZE) new_top_size = MINSIZE;
        top->set_head(ChunkSize{new_top_size}, ChunkFlag::PREV_INUSE);
    }
    return result;
}

} // namespace ptmalloc
} // namespace my_ptmalloc
