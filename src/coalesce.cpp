// CoalescePolicy implementations

#include "my_ptmalloc/coalesce.h"
#include "my_ptmalloc/config.h"

namespace my_ptmalloc {

bool AdaptiveCoalesce::should_merge(ChunkSize cs) noexcept {
    // If high churn (many splits, few merges), skip small chunk merging
    if (split_count_ > merge_count_ * 2 && cs.value < 512) {
        return false;
    }
    return true;
}

} // namespace my_ptmalloc
