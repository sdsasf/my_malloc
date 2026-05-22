#pragma once
// OS-level memory allocation (mmap/munmap/sbrk abstraction).

#include "config.h"
#include "chunk.h"

namespace my_ptmalloc {
namespace ptmalloc {

class SysMemory {
public:
    // Allocate memory via mmap. Returns the chunk at the start.
    // size: total bytes requested (must be page-aligned internally).
    [[nodiscard]] void* map(size_t size, size_t alignment = MALLOC_ALIGNMENT) noexcept;

    // Release memory via munmap.
    void unmap(Chunk* p, size_t size) noexcept;

    // Release a sub-region within a mapping (via munmap partial).
    void unmap_region(void* addr, size_t size) noexcept;

    // Trim: release trailing unused pages from a region via madvise.
    // Returns true if any memory was released.
    bool trim(void* start, size_t current_size, size_t target_size,
              size_t extra_pad) noexcept;

    // Get the page size.
    static size_t page_size() noexcept;

    // Get mmap'd chunk address + size given a Chunk*.
    // For mmap'd chunks, the mapping starts at the chunk itself.
    static void* mmap_addr(Chunk* p) noexcept { return static_cast<void*>(p); }
};

} // namespace ptmalloc
} // namespace my_ptmalloc
