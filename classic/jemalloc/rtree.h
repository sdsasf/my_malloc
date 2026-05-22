#pragma once
// rtree: multi-level radix tree for address → extent_t lookup.
// Lock-free reads, COW writes.

#include "size_classes.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {
namespace jemalloc {

struct extent_t;  // forward decl

// Simple implementation: single-level array indexed by page number.
// Real jemalloc uses a multi-level radix tree with dynamic depth.
// For learning, a hash-style mapping is sufficient to show the concept.

constexpr size_t RTREE_NSLOTS = 65536;  // 64K slots × ~4KB pages = 256MB lookup range
constexpr size_t RTREE_SHIFT = JE_LG_PAGE;

struct rtree_t {
    std::atomic<extent_t*> slots[RTREE_NSLOTS];

    rtree_t() noexcept;

    // Lookup extent by address
    extent_t* get(void* addr) const noexcept;
    // Set mapping
    void set(void* addr, extent_t* ext) noexcept;
    // Clear mapping
    void clear(void* addr) noexcept;
};

extern rtree_t g_rtree;

} // namespace jemalloc
} // namespace my_ptmalloc
