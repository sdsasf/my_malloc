#pragma once
// OS memory primitives for mimalloc: mmap/munmap, segment allocation.

#include "types.h"

namespace my_ptmalloc {
namespace mimalloc {

struct OS {
    // Allocate a MI_SEGMENT_SIZE-aligned region of memory.
    // Uses mmap with over-allocation and alignment trimming.
    [[nodiscard]] static void* segment_alloc() noexcept;

    // Free a segment
    static void segment_free(void* seg) noexcept;

    // Allocate arbitrary memory via mmap
    [[nodiscard]] static void* raw_alloc(size_t size, size_t alignment = 16) noexcept;

    // Free arbitrary allocation
    static void raw_free(void* ptr, size_t size) noexcept;

    // Page size
    [[nodiscard]] static size_t page_size() noexcept;

    // Reset (decommit) page memory via madvise MADV_DONTNEED
    static void page_reset(void* addr, size_t size) noexcept;
};

} // namespace mimalloc
} // namespace my_ptmalloc
