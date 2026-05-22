// PageMap implementation — 2-level radix tree.

#include "page_map.h"

#include <cstring>
#include <cstdlib>

namespace my_ptmalloc {
namespace tcmalloc {

PageMap::PageMap() noexcept {
    std::memset(root_, 0, sizeof(root_));
}

PageMap::~PageMap() noexcept {
    for (unsigned i = 0; i < kRootSize; i++) {
        if (root_[i]) {
            delete[] root_[i];
            root_[i] = nullptr;
        }
    }
}

Span* PageMap::get(uintptr_t page_number) const noexcept {
    unsigned root_idx = (page_number >> kLeafBits) & (kRootSize - 1);
    Span** leaf = root_[root_idx];
    if (!leaf) return nullptr;
    unsigned leaf_idx = page_number & (kLeafSize - 1);
    return leaf[leaf_idx];
}

void PageMap::set(uintptr_t page_number, Span* span) noexcept {
    unsigned root_idx = (page_number >> kLeafBits) & (kRootSize - 1);
    if (!root_[root_idx]) {
        root_[root_idx] = new Span*[kLeafSize]();
    }
    unsigned leaf_idx = page_number & (kLeafSize - 1);
    root_[root_idx][leaf_idx] = span;
}

void PageMap::map_span(Span* span) noexcept {
    for (size_t i = 0; i < span->length; i++) {
        set(span->start + i, span);
    }
}

} // namespace tcmalloc
} // namespace my_ptmalloc
