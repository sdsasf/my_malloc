#pragma once
// PageMap: 2-level radix tree mapping page_number → Span*.
// O(1) lookup, low memory overhead.

#include "span.h"

namespace my_ptmalloc {
namespace tcmalloc {

// PageMap: root layer of 512 entries, leaf layer of 1024 entries each.
// Covers 512 * 1024 = 524288 pages * 8KB = 4GB of address space
// (adequate for 32-bit address spaces; for 64-bit we'd use 3-level).

constexpr unsigned kRootBits  = 9;   // 512 root entries
constexpr unsigned kLeafBits  = 10;  // 1024 leaf entries per root
constexpr unsigned kRootSize  = 1 << kRootBits;
constexpr unsigned kLeafSize  = 1 << kLeafBits;

struct PageMap {
    // Root: sparse array of pointers to leaf nodes
    Span** root_[kRootSize];

    PageMap() noexcept;
    ~PageMap() noexcept;

    // Get the span for a given page number
    Span* get(uintptr_t page_number) const noexcept;

    // Set the span for a given page number
    void set(uintptr_t page_number, Span* span) noexcept;

    // Get the span for a given pointer
    Span* get_for_ptr(void* p) const noexcept {
        return get(Span::page_number(p));
    }

    // Map all pages in a span
    void map_span(Span* span) noexcept;
};

} // namespace tcmalloc
} // namespace my_ptmalloc
