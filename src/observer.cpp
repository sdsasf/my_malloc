// Observer implementations

#include "my_ptmalloc/observer.h"

namespace my_ptmalloc {

void StatsObserver::on_alloc(size_t size, bool from_mmap) noexcept {
    alloc_count_++;
    total_alloc_bytes_ += size;
    if (from_mmap) mmap_count_++;
}

void StatsObserver::on_free(size_t size, bool to_mmap) noexcept {
    free_count_++;
    total_free_bytes_ += size;
}

void StatsObserver::on_trim(size_t) noexcept {
    trim_count_++;
}

void StatsObserver::on_consolidate(size_t) noexcept {
    consolidate_count_++;
}

} // namespace my_ptmalloc
