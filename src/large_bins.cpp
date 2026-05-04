// LargeBins implementation: best-fit search via sorted fd/bk list
// NOTE: fd_nextsize/bk_nextsize chain is NOT maintained because consolidation
// can corrupt it (chunk memory is reused as user data when allocated, and
// consolidate_and_free doesn't update large bin chains). Linear search over
// the sorted fd/bk list is correct and sufficient.

#include "my_ptmalloc/large_bins.h"
#include "my_ptmalloc/unsorted_bin.h"
#include <algorithm>

namespace my_ptmalloc {

Chunk* LargeBins::alloc_bestfit(ChunkSize nb) noexcept {
    size_t offset = bin_offset(largebin_index(nb));
    if (offset >= NUM_LARGE_BINS) return nullptr;

    size_t abs_idx = NSMALLBINS + 2 + offset;
    while (true) {
        auto next = binmap_.find_first_from(abs_idx);
        if (!next || *next >= NBINS) return nullptr;

        size_t i = bin_offset(LargebinIdx{*next});
        if (i >= NUM_LARGE_BINS) return nullptr;
        IntrusiveList& bin = bins_[i];
        if (bin.empty()) {
            binmap_.clear(*next);
            abs_idx = *next + 1;
            continue;
        }

        // Walk from back (smallest) to front (largest) for best-fit
        Chunk* p = bin.back();
        Chunk* sentinel = bin.sentinel();
        while (p != sentinel) {
            if (p->chunk_size().value >= nb.value) {
                bin.unlink(p);
                if (bin.empty()) binmap_.clear(*next);
                p->fd_nextsize = nullptr;
                p->bk_nextsize = nullptr;
                return p;
            }
            p = p->bk;
        }

        abs_idx = *next + 1;
    }
}

void LargeBins::free_sorted(Chunk* p) noexcept {
    LargebinIdx idx = largebin_index(p->chunk_size());
    size_t offset = bin_offset(idx);
    if (offset >= NUM_LARGE_BINS) offset = NUM_LARGE_BINS - 1;

    IntrusiveList& bin = bins_[offset];
    Chunk* sentinel = bin.sentinel();
    binmap_.mark(NSMALLBINS + 2 + offset);

    // Find insertion point: sorted by size descending (front = largest)
    Chunk* fwd = bin.front();

    if (fwd == sentinel) {
        // Empty bin: just insert
        bin.push_back(p);
        p->fd_nextsize = nullptr;
        p->bk_nextsize = nullptr;
        return;
    }

    // Walk forward (largest to smallest) to find insertion point
    while (fwd != sentinel && fwd->chunk_size().value > p->chunk_size().value) {
        fwd = fwd->fd;
    }

    if (fwd == sentinel) {
        // Insert at back (smallest)
        bin.push_back(p);
    } else if (fwd->chunk_size().value == p->chunk_size().value) {
        // Same size class: insert after fwd
        p->fd = fwd->fd;
        p->bk = fwd;
        fwd->fd->bk = p;
        fwd->fd = p;
    } else {
        // New size class: insert before fwd (which is smaller)
        p->fd = fwd;
        p->bk = fwd->bk;
        fwd->bk->fd = p;
        fwd->bk = p;
    }

    // fd_nextsize/bk_nextsize are not maintained (see comment at top)
    p->fd_nextsize = nullptr;
    p->bk_nextsize = nullptr;
}

std::pair<Chunk*, Chunk*> LargeBins::alloc_split(ChunkSize nb, UnsortedBin& ub) noexcept {
    Chunk* victim = alloc_bestfit(nb);
    if (!victim) return {nullptr, nullptr};

    size_t victim_size = victim->chunk_size().value;
    size_t remainder_size = victim_size - nb.value;

    if (remainder_size >= MINSIZE) {
        // Split: carve nb from victim, remainder to unsorted
        ChunkFlag flags = victim->flags();
        Chunk* remainder = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(victim) + nb.value);
        remainder->prev_size = 0;
        remainder->set_head(ChunkSize{remainder_size}, flags | ChunkFlag::PREV_INUSE);
        // Update the chunk after remainder: its prev_size must reflect
        // the remainder's size (not the original victim's size)
        remainder->set_foot(ChunkSize{remainder_size});
        victim->set_size(nb);

        ub.push(remainder);
        return {victim, remainder};
    }

    // No split possible, use entire chunk
    return {victim, nullptr};
}

} // namespace my_ptmalloc
