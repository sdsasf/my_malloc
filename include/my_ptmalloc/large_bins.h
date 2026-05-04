#pragma once
// LargeBins: 62 size-range bins with sorted order and binmap skipping
// Bins 66-127 (index 0-61 in our array), logarithmically spaced
// Sorted by size (largest first). fd_nextsize/bk_nextsize are cleared but
// not maintained as an active secondary chain.

#include "config.h"
#include "types.h"
#include "chunk.h"
#include "intrusive_list.h"
#include "bin_map.h"
#include <utility>

namespace my_ptmalloc {

// Forward declaration
class UnsortedBin;

class LargeBins {
    // 62 bins: NBINS - NSMALLBINS - 2 (subtract unsorted bin 1 and unused bin 0)
    static constexpr size_t NUM_LARGE_BINS = NBINS - NSMALLBINS - 2;
    IntrusiveList bins_[NUM_LARGE_BINS];
    BinMap binmap_;

public:
    void init() noexcept {
        for (auto& b : bins_) b.init();
        binmap_.init();
    }

    // Best-fit search: find smallest chunk >= requested size
    // Returns nullptr if no suitable chunk found
    [[nodiscard]] Chunk* alloc_bestfit(ChunkSize nb) noexcept;

    // Insert chunk in sorted order (largest first within bin)
    void free_sorted(Chunk* p) noexcept;

    // Alloc with split: carve nb from victim, remainder to unsorted bin
    [[nodiscard]] std::pair<Chunk*, Chunk*> alloc_split(
        ChunkSize nb, UnsortedBin& ub) noexcept;

    // Get internal bin index from largebin_index
    [[nodiscard]] static size_t bin_offset(LargebinIdx idx) noexcept {
        return idx.value - (NSMALLBINS + 2);  // Map bin 66-127 to 0-61
    }

};

} // namespace my_ptmalloc
