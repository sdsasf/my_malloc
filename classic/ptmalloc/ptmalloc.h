#pragma once
// ptmalloc public API — glibc-style memory allocator.

#include "config.h"

#include <cstddef>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Core API ───
[[nodiscard]] void* pt_malloc(size_t size) noexcept;
void  pt_free(void* ptr) noexcept;
[[nodiscard]] void* pt_calloc(size_t n, size_t size) noexcept;
[[nodiscard]] void* pt_realloc(void* ptr, size_t size) noexcept;

// ─── Aligned API ───
[[nodiscard]] void* pt_memalign(size_t alignment, size_t size) noexcept;
[[nodiscard]] size_t pt_malloc_usable_size(void* ptr) noexcept;

// ─── Init ───
void pt_malloc_init() noexcept;

} // namespace ptmalloc
} // namespace my_ptmalloc
