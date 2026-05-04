#pragma once
// BinManager: facade composing all bin types
// Provides unified access to fastbins, small bins, large bins, unsorted bin, and binmap

#include "fastbins.h"
#include "small_bins.h"
#include "large_bins.h"
#include "unsorted_bin.h"
#include "bin_map.h"

namespace my_ptmalloc {

class BinManager {
    FastBins     fastbins_;
    SmallBins    smallbins_;
    LargeBins    largebins_;
    UnsortedBin  unsorted_;
    BinMap       binmap_;

public:
    void init() noexcept {
        fastbins_.init();
        smallbins_.init();
        largebins_.init();
        unsorted_.init();
        binmap_.init();
    }

    [[nodiscard]] FastBins&    fast()    noexcept { return fastbins_; }
    [[nodiscard]] SmallBins&   small()   noexcept { return smallbins_; }
    [[nodiscard]] LargeBins&   large()   noexcept { return largebins_; }
    [[nodiscard]] UnsortedBin& unsorted() noexcept { return unsorted_; }
    [[nodiscard]] BinMap&      map()     noexcept { return binmap_; }
};

} // namespace my_ptmalloc
