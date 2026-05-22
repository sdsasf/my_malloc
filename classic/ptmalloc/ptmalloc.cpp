// ptmalloc core implementation: malloc/free/realloc/calloc/memalign.
// Faithful to glibc's allocation algorithm.

#include "ptmalloc.h"
#include "arena.h"
#include "arena_manager.h"
#include "bins.h"
#include "chunk.h"
#include "coalesce.h"
#include "heap.h"
#include "sys_memory.h"
#include "tcache.h"

#include <pthread.h>
#include <cstring>
#include <cerrno>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Init ───
static pthread_once_t g_ptmalloc_init_once = PTHREAD_ONCE_INIT;

static void pt_malloc_init_once() noexcept {
    static ArenaManager arena_mgr;
    g_arena_manager = &arena_mgr;
    g_arena_manager->init();
}

void pt_malloc_init() noexcept {
    pthread_once(&g_ptmalloc_init_once, pt_malloc_init_once);
}

// ─── Allocation pipeline ───
// Strategy order (same as glibc):
//   1. Tcache (lock-free, fast path)
//   2. Fastbins (CAS lock-free)
//   3. Smallbins (exact-fit FIFO)
//   4. Unsorted bin scan + sort (consolidate fastbins first)
//   5. Largebins (best-fit)
//   6. Top chunk split
//   7. SysAlloc (mmap or extend heap)

void* pt_malloc(size_t size) noexcept {
    pt_malloc_init();

    if (size == 0) size = 1;

    // Compute chunk size
    ChunkSize nb = request2size(UserSize{size});

    // ── Strategy 1: Tcache ──
    if (tcache == nullptr) tcache_init();
    if (tcache) {
        TcacheIdx tidx = csize2tidx(nb);
        if (tidx.value < TCACHE_MAX_BINS) {
            void* cached = tcache->alloc(tidx);
            if (cached) {
                Chunk* c = Chunk::from_user_ptr(cached);
                // Validate
                size_t cs = c->chunk_size();
                if (cs >= MINSIZE && (cs & MALLOC_ALIGN_MASK) == 0) {
                    c->mark_inuse();
                    return cached;
                }
                // Corrupt tcache chunk — discard silently
            }
        }
    }

    // ── Strategy 2: Fastbins ──
    // Fastbins require arena lock for the bin operations, but we can use CAS.
    // For simplicity we take the arena lock.
    if (nb.value <= MAX_FAST_SIZE) {
        // Ensure thread has an arena
        if (thread_arena == nullptr) {
            if (!g_arena_manager->get_main_arena()->trylock()) {
                g_arena_manager->get_main_arena()->lock();
            }
        }
        Arena* av = g_arena_manager->get_arena(nb.value);

        FastbinIdx fidx = fastbin_index(nb);
        if (fidx.value < NFASTBINS) {
            Chunk* fp = av->bins_.fast(fidx).pop(fidx);
            if (fp) {
                size_t cs = fp->chunk_size();
                if (cs >= MINSIZE && (cs & MALLOC_ALIGN_MASK) == 0) {
                    fp->mark_inuse();
                    av->unlock();
                    return fp->user_data();
                }
            }
        }
        av->unlock();
    }

    // ── Strategy 3-7: Arena-based allocation ──
    // Get arena (locked)
    if (thread_arena == nullptr) {
        Arena* av = g_arena_manager->get_arena(nb.value);
        if (!av) return nullptr;
        thread_arena = av;
    } else {
        // Try our existing arena
        if (!thread_arena->trylock()) {
            // Contention — use main or new arena
            Arena* av = g_arena_manager->get_arena(nb.value);
            thread_arena = av;
        }
    }

    Arena* av = thread_arena;
    void* result = nullptr;

    // ── Strategy 3: Smallbins ──
    if (nb.value < MIN_LARGE_SIZE) {
        SmallbinIdx sidx = smallbin_index(nb);
        Chunk* p = av->bins_.alloc(sidx);
        if (p) {
            p->mark_inuse();
            result = p->user_data();
            goto done;
        }
    }

    // ── Strategy 4: Unsorted bin scan ──
    // Consolidate fastbins first if needed
    if (av->has_fastchunks()) {
        malloc_consolidate(*av);
    }

    // Scan unsorted bin
    {
        BinList& unsorted = av->bins_.unsorted();
        Chunk* sentinel = unsorted.as_chunk();
        Chunk* p = unsorted.fd;

        while (p != sentinel && p) {
            Chunk* next_p = p->fd;
            size_t psize = p->chunk_size();

            // Check if exact fit in smallbin
            if (in_smallbin_range(ChunkSize{psize}) && psize == nb.value) {
                av->bins_.unlink_free_chunk(p);
                p->mark_inuse();
                result = p->user_data();
                goto done;
            }

            // If size matches, take it
            if (psize >= nb.value) {
                // Best-fit: first adequate chunk in unsorted bin
                av->bins_.unlink_free_chunk(p);

                size_t remainder_size = psize - nb.value;
                ChunkFlag flags = p->flags() | ChunkFlag::PREV_INUSE;

                if (remainder_size >= MINSIZE) {
                    // Split
                    p->set_head(nb, flags);
                    Chunk* remainder = reinterpret_cast<Chunk*>(
                        reinterpret_cast<uintptr_t>(p) + nb.value);
                    remainder->prev_size = nb.value;
                    remainder->set_head(ChunkSize{remainder_size}, flags);
                    remainder->set_foot(ChunkSize{remainder_size});

                    // Place remainder in appropriate bin
                    if (in_smallbin_range(ChunkSize{remainder_size})) {
                        SmallbinIdx si = smallbin_index(ChunkSize{remainder_size});
                        av->bins_.small(si).push_front(remainder);
                        av->bins_.mark_bin(UNSORTED_BIN_IDX + 1 + si.value);
                    } else {
                        unsigned li = av->bins_.largebin_index(ChunkSize{remainder_size});
                        av->bins_.large(li).push_front(remainder);
                        av->bins_.mark_bin(UNSORTED_BIN_IDX + 1 + NSMALLBINS + li);
                    }
                } else {
                    p->set_head(ChunkSize{psize}, flags);
                }

                p->mark_inuse();
                result = p->user_data();
                goto done;
            }

            // Sort this unsorted chunk into its correct bin
            if (in_smallbin_range(ChunkSize{psize})) {
                SmallbinIdx si = smallbin_index(ChunkSize{psize});
                av->bins_.unsorted().unlink(p);
                av->bins_.small(si).push_front(p);
                av->bins_.mark_bin(UNSORTED_BIN_IDX + 1 + si.value);
            } else {
                av->bins_.unsorted().unlink(p);
                unsigned li = av->bins_.largebin_index(ChunkSize{psize});
                av->bins_.large(li).push_front(p);
                av->bins_.mark_bin(UNSORTED_BIN_IDX + 1 + NSMALLBINS + li);
            }

            p = next_p;
        }

        // Unsorted bin is now empty (or only has newly added entries we'll re-scan)
        av->bins_.unmark_bin(UNSORTED_BIN_IDX);
    }

    // ── Strategy 5: Largebins ──
    if (nb.value >= MIN_LARGE_SIZE) {
        auto [victim, remainder] = av->bins_.alloc_split(nb, av->bins_.unsorted());
        if (victim) {
            victim->mark_inuse();
            if (remainder) av->set_last_remainder(remainder);
            result = victim->user_data();
            goto done;
        }
    }

    // ── Strategy 6: Top chunk ──
    {
        Chunk* top = av->top();
        if (top && top->chunk_size() >= nb.value) {
            size_t top_size = top->chunk_size();
            size_t remainder_size = top_size - nb.value;
            bool non_main = !av->is_main_;

            if (remainder_size >= MINSIZE) {
                // Split top
                Chunk* new_top = reinterpret_cast<Chunk*>(
                    reinterpret_cast<uintptr_t>(top) + nb.value);
                new_top->prev_size = nb.value;
                new_top->set_head(ChunkSize{remainder_size}, ChunkFlag::PREV_INUSE);
                if (non_main) new_top->size |= NON_MAIN_ARENA_BIT;
                av->set_top(new_top);
                top->set_head(nb, ChunkFlag::PREV_INUSE);
            } else {
                // Use entire top chunk
                av->set_top(nullptr);
                top->set_head(ChunkSize{top_size}, ChunkFlag::PREV_INUSE);
            }

            if (non_main) top->size |= NON_MAIN_ARENA_BIT;
            result = top->user_data();
            goto done;
        }
    }

    // ── Strategy 7: SysAlloc ──
    {
        SysMemory* mem = g_arena_manager->sys_memory();
        ThresholdPolicy* thresh = g_arena_manager->threshold();
        size_t mmap_thresh = thresh->mmap_threshold();
        bool non_main = !av->is_main_;

        if (nb.value >= mmap_thresh) {
            // Direct mmap
            size_t alloc_size = nb.value + CHUNK_HDR_SZ;
            alloc_size = (alloc_size + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;

            void* raw = mem->map(alloc_size);
            if (!raw) goto done;

            Chunk* chunk = static_cast<Chunk*>(raw);
            chunk->prev_size = 0;
            chunk->set_head(ChunkSize{alloc_size},
                ChunkFlag::PREV_INUSE | ChunkFlag::IS_MMAPPED);

            thresh->adjust_mmap_threshold(true);
            result = chunk->user_data();
            goto done;
        }

        // Extend heap
        size_t extend_size = nb.value + 2 * MINSIZE;
        if (extend_size < HEAP_MAX_SIZE) extend_size = HEAP_MAX_SIZE;
        extend_size = (extend_size + HEAP_MAX_SIZE - 1) & ~(HEAP_MAX_SIZE - 1);

        if (non_main) {
            HeapInfo* h = HeapInfo::new_heap(extend_size, av, mem);
            if (!h) goto done;

            uintptr_t addr = reinterpret_cast<uintptr_t>(h) + sizeof(HeapInfo);
            addr = (addr + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;
            size_t prefix = addr - reinterpret_cast<uintptr_t>(h);
            if (extend_size <= prefix + MINSIZE) goto done;

            size_t usable = extend_size - prefix - MINSIZE;

            Chunk* chunk = reinterpret_cast<Chunk*>(addr);
            chunk->prev_size = 0;

            // Fencepost at end
            Chunk* fence = reinterpret_cast<Chunk*>(addr + usable);
            fence->prev_size = usable;
            fence->set_head(ChunkSize{MINSIZE},
                ChunkFlag::PREV_INUSE | ChunkFlag::NON_MAIN_ARENA);

            size_t remainder = usable - nb.value;
            if (remainder >= MINSIZE) {
                chunk->set_head(nb, ChunkFlag::PREV_INUSE | ChunkFlag::NON_MAIN_ARENA);
                Chunk* new_top = reinterpret_cast<Chunk*>(addr + nb.value);
                new_top->prev_size = 0;
                new_top->set_head(ChunkSize{remainder},
                    ChunkFlag::PREV_INUSE | ChunkFlag::NON_MAIN_ARENA);

                if (av->top()) {
                    av->bins_.unsorted().push_front(av->top());
                    av->bins_.mark_bin(UNSORTED_BIN_IDX);
                }
                av->set_top(new_top);
            } else {
                chunk->set_head(ChunkSize{usable},
                    ChunkFlag::PREV_INUSE | ChunkFlag::NON_MAIN_ARENA);
            }

            av->update_system_mem(h->size);
            thresh->adjust_mmap_threshold(false);
            result = chunk->user_data();
            goto done;
        } else {
            // Main arena — mmap heap extension
            void* heap = mem->map(extend_size);
            if (!heap) goto done;

            Chunk* chunk = static_cast<Chunk*>(heap);
            chunk->prev_size = 0;

            size_t usable = extend_size - MINSIZE;
            Chunk* fence = reinterpret_cast<Chunk*>(
                reinterpret_cast<uintptr_t>(heap) + usable);
            fence->prev_size = usable;
            fence->set_head(ChunkSize{MINSIZE}, ChunkFlag::PREV_INUSE);

            size_t remainder = usable - nb.value;
            if (remainder >= MINSIZE) {
                chunk->set_head(nb, ChunkFlag::PREV_INUSE);
                Chunk* new_top = reinterpret_cast<Chunk*>(
                    reinterpret_cast<uintptr_t>(heap) + nb.value);
                new_top->prev_size = 0;
                new_top->set_head(ChunkSize{remainder}, ChunkFlag::PREV_INUSE);

                if (av->top()) {
                    av->bins_.unsorted().push_front(av->top());
                    av->bins_.mark_bin(UNSORTED_BIN_IDX);
                }
                av->set_top(new_top);
            } else {
                chunk->set_head(ChunkSize{usable}, ChunkFlag::PREV_INUSE);
            }

            av->update_system_mem(extend_size);
            thresh->adjust_mmap_threshold(false);
            result = chunk->user_data();
            goto done;
        }
    }

done:
    av->unlock();
    return result;
}

// ─── Free ───
void pt_free(void* ptr) noexcept {
    if (!ptr) return;

    Chunk* p = Chunk::from_user_ptr(ptr);
    size_t chunk_size = p->chunk_size();

    // Validate
    if (chunk_size < MINSIZE || (chunk_size & MALLOC_ALIGN_MASK) != 0) {
        PTMALLOC_LOG("free: corrupt header %p size=0x%zx", ptr, chunk_size);
        return;
    }

    // If mmap'd, unmap directly
    if (p->is_mmapped()) {
        if (g_arena_manager) {
            SysMemory* mem = g_arena_manager->sys_memory();
            if (mem) mem->unmap(p, chunk_size);
        }
        return;
    }

    // ── Try tcache ──
    if (tcache && in_tcache_range(ChunkSize{chunk_size})) {
        TcacheIdx tidx = csize2tidx(ChunkSize{chunk_size});
        if (tidx.value < TCACHE_MAX_BINS) {
            // Double-free detection
            auto* e = reinterpret_cast<TcacheEntry*>(p->user_data());
            if (tcache->is_double_free(e)) {
                PTMALLOC_LOG("free: double free detected %p", ptr);
                return;
            }
            if (tcache->free(tidx, p)) {
                return;
            }
        }
    }

    // ── Try fastbin ──
    if (chunk_size <= MAX_FAST_SIZE) {
        // Need arena for fastbin push
        Arena* arena = HeapInfo::arena_for_chunk(p);
        if (!arena) arena = g_arena_manager->get_main_arena();

        FastbinIdx fidx = fastbin_index(ChunkSize{chunk_size});
        if (fidx.value < NFASTBINS) {
            // Set flag: chunk's PREV_INUSE stays set so it won't be coalesced
            // while in fastbins
            ArenaGuard guard(arena);
            arena->bins_.fast(fidx).push(p);
            return;
        }
    }

    // ── Arena-based free with coalescing ──
    Arena* arena = HeapInfo::arena_for_chunk(p);
    if (!arena) {
        arena = g_arena_manager->get_main_arena();
    }

    arena->lock();
    // Clear prev_inuse on next chunk (mark this chunk as free)
    p->clear_previnuse();
    consolidate_and_free(*arena, p);
    arena->unlock();
}

// ─── Calloc ───
void* pt_calloc(size_t n, size_t size) noexcept {
    // Overflow check
    size_t total = n * size;
    if (n != 0 && total / n != size) return nullptr;

    void* p = pt_malloc(total);
    if (p) {
        size_t usable = pt_malloc_usable_size(p);
        if (usable > total) usable = total;
        std::memset(p, 0, usable);
    }
    return p;
}

// ─── Realloc ───
void* pt_realloc(void* ptr, size_t size) noexcept {
    if (!ptr) return pt_malloc(size);
    if (size == 0) {
        pt_free(ptr);
        return nullptr;
    }

    Chunk* oldp = Chunk::from_user_ptr(ptr);
    size_t old_size = oldp->chunk_size();
    size_t old_usable = old_size - SIZE_SZ;  // prev_size of next chunk is usable

    ChunkSize nb = request2size(UserSize{size});

    // If current chunk is big enough, stay as-is
    if (old_size >= nb.value) return ptr;

    if (!oldp->is_mmapped()) {
        Arena* arena = HeapInfo::arena_for_chunk(oldp);
        if (!arena) arena = g_arena_manager->get_main_arena();

        arena->lock();
        ChunkFlag flags = oldp->flags();
        Chunk* next = oldp->next_chunk();

        // Try to extend into top chunk
        if (next == arena->top()) {
            size_t total = old_size + next->chunk_size();
            if (total >= nb.value) {
                size_t remainder_size = total - nb.value;
                if (remainder_size >= MINSIZE) {
                    Chunk* new_top = reinterpret_cast<Chunk*>(
                        reinterpret_cast<uintptr_t>(oldp) + nb.value);
                    new_top->prev_size = nb.value;
                    new_top->set_head(ChunkSize{remainder_size},
                        flags | ChunkFlag::PREV_INUSE);
                    new_top->set_foot(ChunkSize{remainder_size});
                    arena->set_top(new_top);
                    oldp->set_head(nb, flags);
                } else {
                    arena->set_top(nullptr);
                    oldp->set_head(ChunkSize{total}, flags);
                    oldp->mark_inuse();
                }
                arena->unlock();
                return ptr;
            }
        }

        // Try to extend into next free chunk
        if (!next->prev_inuse() && arena->bins_.contains_free_chunk(next)) {
            size_t next_size = next->chunk_size();
            size_t total = old_size + next_size;
            if (total >= nb.value && arena->bins_.unlink_free_chunk(next)) {
                if (arena->last_remainder_ == next)
                    arena->set_last_remainder(nullptr);

                size_t remainder_size = total - nb.value;
                if (remainder_size >= MINSIZE) {
                    oldp->set_head(nb, flags);
                    Chunk* remainder = reinterpret_cast<Chunk*>(
                        reinterpret_cast<uintptr_t>(oldp) + nb.value);
                    remainder->prev_size = nb.value;
                    remainder->set_head(ChunkSize{remainder_size},
                        flags | ChunkFlag::PREV_INUSE);
                    remainder->set_foot(ChunkSize{remainder_size});
                    remainder->clear_previnuse();
                    arena->bins_.unsorted().push_front(remainder);
                    arena->bins_.mark_bin(UNSORTED_BIN_IDX);
                } else {
                    oldp->set_head(ChunkSize{total}, flags);
                    oldp->mark_inuse();
                }
                arena->unlock();
                return ptr;
            }
        }

        arena->unlock();
    }

    // Fallback: allocate-copy-free
    void* new_ptr = pt_malloc(size);
    if (!new_ptr) return nullptr;
    size_t copy_size = old_usable < size ? old_usable : size;
    std::memcpy(new_ptr, ptr, copy_size);
    pt_free(ptr);
    return new_ptr;
}

// ─── Memalign ───
void* pt_memalign(size_t alignment, size_t size) noexcept {
    if (alignment < sizeof(void*)) alignment = sizeof(void*);
    if ((alignment & (alignment - 1)) != 0) {
        // Round up to next power of 2
        alignment--;
        alignment |= alignment >> 1;
        alignment |= alignment >> 2;
        alignment |= alignment >> 4;
        alignment |= alignment >> 8;
        alignment |= alignment >> 16;
        alignment |= alignment >> 32;
        alignment++;
        if (alignment == 0) return nullptr;
    }
    if (alignment <= MALLOC_ALIGNMENT) return pt_malloc(size);

    ChunkSize nb = request2size(UserSize{size});

    // Over-allocate to guarantee alignment: nb + 2*alignment + CHUNK_HDR_SZ
    size_t alloc_size = nb.value + 2 * alignment + CHUNK_HDR_SZ;
    void* raw = pt_malloc(alloc_size);
    if (!raw) return nullptr;

    Chunk* p = Chunk::from_user_ptr(raw);
    size_t psize = p->chunk_size();

    if ((reinterpret_cast<uintptr_t>(raw) & (alignment - 1)) == 0) {
        return raw;  // already aligned
    }

    uintptr_t raw_addr   = reinterpret_cast<uintptr_t>(raw);
    uintptr_t aligned    = (raw_addr + alignment - 1) & ~(alignment - 1);
    Chunk* aligned_chunk = Chunk::from_user_ptr(reinterpret_cast<void*>(aligned));
    uintptr_t front_size = reinterpret_cast<uintptr_t>(aligned_chunk)
                         - reinterpret_cast<uintptr_t>(p);

    // If front piece too small, advance to next alignment boundary
    if (front_size < MINSIZE) {
        aligned       += alignment;
        aligned_chunk  = Chunk::from_user_ptr(reinterpret_cast<void*>(aligned));
        front_size     = reinterpret_cast<uintptr_t>(aligned_chunk)
                       - reinterpret_cast<uintptr_t>(p);
    }

    size_t remainder = psize - front_size;

    // Set up aligned chunk
    aligned_chunk->set_head(ChunkSize{remainder}, ChunkFlag::PREV_INUSE);
    aligned_chunk->mark_inuse();

    // Set up and free the front piece
    p->set_head(ChunkSize{front_size}, ChunkFlag::PREV_INUSE);
    pt_free(p->user_data());

    return reinterpret_cast<void*>(aligned);
}

// ─── Usable size ───
size_t pt_malloc_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;
    Chunk* p = Chunk::from_user_ptr(ptr);
    size_t cs = p->chunk_size();
    // Usable = chunk_size - SIZE_SZ (next chunk's prev_size is usable)
    return cs > SIZE_SZ ? cs - SIZE_SZ : 0;
}

} // namespace ptmalloc
} // namespace my_ptmalloc
