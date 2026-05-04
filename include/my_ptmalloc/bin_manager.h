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

    [[nodiscard]] bool contains_free_chunk(Chunk* p) noexcept {
        if (!p || !p->is_valid()) return false;
        if (unsorted_.contains(p)) return true;
        if (in_smallbin_range(p->chunk_size())) {
            return smallbins_.contains(smallbin_index(p->chunk_size()), p);
        }
        return largebins_.contains(p);
    }

    bool unlink_free_chunk(Chunk* p) noexcept {
        if (!p || !p->is_valid() || !p->fd || !p->bk ||
            p->fd->bk != p || p->bk->fd != p) {
            return false;
        }

        if (unsorted_.contains(p)) {
            unsorted_.unlink(p);
            return true;
        }

        if (in_smallbin_range(p->chunk_size())) {
            SmallbinIdx idx = smallbin_index(p->chunk_size());
            if (!smallbins_.contains(idx, p)) return false;
            smallbins_.unlink(idx, p);
            return true;
        }

        return largebins_.unlink(p);
    }
};

} // namespace my_ptmalloc
