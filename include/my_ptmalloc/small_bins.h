#pragma once
// SmallBins: 64 exact-fit doubly-linked FIFO bins
// Size range: MINSIZE to MIN_LARGE_SIZE (1024 on 64-bit)
// Each bin holds chunks of exactly one size class

#include "config.h"
#include "types.h"
#include "chunk.h"
#include "intrusive_list.h"
#include <cstdio>

namespace my_ptmalloc {

class SmallBins {
    IntrusiveList bins_[NSMALLBINS];

public:
    void init() noexcept {
        for (auto& b : bins_) b.init();
    }

    // FIFO alloc: take from back
    [[nodiscard]] Chunk* alloc(SmallbinIdx idx) noexcept {
        IntrusiveList& bin = bins_[idx.value];
        if (bin.empty()) return nullptr;
        Chunk* victim = bin.back();
        bin.unlink(victim);
        return victim;
    }

    // Free: push to front
    void free(SmallbinIdx idx, Chunk* p) noexcept {
        if (!p->is_valid()) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[SmallBins::free] SKIP invalid chunk %p raw=0x%zx prev=0x%zx\n",
                    (void*)p, p->size, p->prev_size);
#endif
            return;
        }
        bins_[idx.value].push_front(p);
    }

    [[nodiscard]] bool has_chunks(SmallbinIdx idx) const noexcept {
        return !bins_[idx.value].empty();
    }

    // Get raw list for direct manipulation
    [[nodiscard]] IntrusiveList& get_bin(SmallbinIdx idx) noexcept {
        return bins_[idx.value];
    }
};

} // namespace my_ptmalloc
