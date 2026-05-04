// ThresholdPolicy implementations

#include "my_ptmalloc/threshold.h"
#include <algorithm>

namespace my_ptmalloc {

void AdaptiveThreshold::on_mmap_alloc(size_t size) noexcept {
    // Increase mmap threshold when mmap'd chunks are freed quickly
    // (indicates transient large allocations)
    if (size > mmap_thresh_ && mmap_thresh_ < size_t(4 * 1024 * 1024)) {
        mmap_thresh_ = std::min(size, size_t(4 * 1024 * 1024));
    }
}

void AdaptiveThreshold::on_mmap_free(size_t size) noexcept {
    // Decrease threshold if small mmap'd chunks are being freed
    if (size < mmap_thresh_ / 2 && mmap_thresh_ > size_t(DEFAULT_MMAP_THRESHOLD)) {
        mmap_thresh_ = std::max(mmap_thresh_ / 2, size_t(DEFAULT_MMAP_THRESHOLD));
    }
}

} // namespace my_ptmalloc
