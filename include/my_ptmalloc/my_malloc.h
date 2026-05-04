#pragma once
// Public C++ API for my_ptmalloc allocator

#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {

// Core allocation functions
[[nodiscard]] void* my_malloc(size_t size) noexcept;
void  my_free(void* ptr) noexcept;
[[nodiscard]] void* my_calloc(size_t n, size_t size) noexcept;
[[nodiscard]] void* my_realloc(void* ptr, size_t size) noexcept;

// Aligned allocation
[[nodiscard]] void* my_memalign(size_t alignment, size_t size) noexcept;
[[nodiscard]] int   my_posix_memalign(void** memptr, size_t alignment, size_t size) noexcept;
[[nodiscard]] void* my_aligned_alloc(size_t alignment, size_t size) noexcept;

// Tuning
int    my_mallopt(int param, int value) noexcept;
size_t my_malloc_usable_size(void* ptr) noexcept;

// Init/shutdown
void   my_malloc_init() noexcept;

} // namespace my_ptmalloc
