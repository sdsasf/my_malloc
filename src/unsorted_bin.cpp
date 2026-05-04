// UnsortedBin implementation: scan and sort chunks into proper bins

#include "my_ptmalloc/unsorted_bin.h"
#include "my_ptmalloc/small_bins.h"
#include "my_ptmalloc/large_bins.h"
#include "my_ptmalloc/tcache.h"
#include "my_ptmalloc/config.h"
#include <cstdio>
#include <cstdint>

namespace my_ptmalloc {

Chunk* UnsortedBin::scan_and_sort(
    ChunkSize nb, SmallBins& sb, LargeBins& lb,
    void* tcache_ptr, Chunk*& last_remainder) noexcept {

    TcachePerthread* tc = reinterpret_cast<TcachePerthread*>(tcache_ptr);
    size_t scanned = 0;
    Chunk* cached_exact = nullptr;

    if (last_remainder) {
        size_t rsize = last_remainder->chunk_size().value;
        if (rsize >= nb.value && last_remainder->fd && last_remainder->bk &&
            last_remainder->fd->bk == last_remainder &&
            last_remainder->bk->fd == last_remainder) {
            Chunk* victim = last_remainder;
            last_remainder = nullptr;
            bin_.unlink(victim);
            size_t remainder_size = rsize - nb.value;
            if (remainder_size >= MINSIZE) {
                ChunkFlag flags = victim->flags();
                Chunk* remainder = reinterpret_cast<Chunk*>(
                    reinterpret_cast<uintptr_t>(victim) + nb.value);
                remainder->prev_size = 0;
                remainder->set_head(ChunkSize{remainder_size}, flags | ChunkFlag::PREV_INUSE);
                remainder->set_foot(ChunkSize{remainder_size});
                victim->set_size(nb);
                last_remainder = remainder;
                bin_.push_front(remainder);
            }
            return victim;
        }
        last_remainder = nullptr;
    }

    while (!bin_.empty() && scanned++ < UNSORTED_SCAN_LIMIT) {
        Chunk* victim = bin_.back();
        size_t victim_size = victim->chunk_size().value;

        // Defensive: validate chunk before processing
        if (victim_size < MINSIZE || (victim_size & MALLOC_ALIGN_MASK) != 0) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[scan_and_sort] CORRUPT chunk in unsorted bin: %p raw=0x%zx prev_size=0x%zx\n",
                    (void*)victim, victim->size, victim->prev_size);
            // Dump the full 48-byte header
            uintptr_t addr = reinterpret_cast<uintptr_t>(victim);
            for (size_t off = 0; off < 48; off += 8) {
                size_t val = *reinterpret_cast<size_t*>(addr + off);
                fprintf(stderr, "  [+%zu] = 0x%zx\n", off, val);
            }
#endif
            bin_.unlink(victim);
            continue;
        }

        // Unlink from unsorted bin
        bin_.unlink(victim);
        if (victim == last_remainder) last_remainder = nullptr;

        // Exact match: return immediately
        if (victim_size == nb.value) {
            if (tc && in_tcache_range(ChunkSize{victim_size})) {
                TcacheIdx tidx = csize2tidx(ChunkSize{victim_size});
                if (tidx.value < TCACHE_MAX_BINS && tc->counts_[tidx.value] < TCACHE_FILL_COUNT) {
                    victim->mark_inuse();
                    if (tc->free(tidx, victim)) {
                        cached_exact = victim;
                        continue;
                    }
                }
            }
            return victim;
        }

        // Place into appropriate bin
        if (in_smallbin_range(ChunkSize{victim_size})) {
            SmallbinIdx idx = smallbin_index(ChunkSize{victim_size});
            sb.free(idx, victim);
        } else {
            lb.free_sorted(victim);
        }
    }

    if (cached_exact && tc) {
        TcacheIdx tidx = csize2tidx(nb);
        void* mem = tc->alloc(tidx);
        if (mem) return Chunk::from_user_ptr(mem);
    }

    return nullptr;
}

} // namespace my_ptmalloc
