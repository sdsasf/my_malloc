#pragma once
// tcmalloc public API.

#include "size_classes.h"

namespace my_ptmalloc {
namespace tcmalloc {

[[nodiscard]] void* tc_malloc(size_t size) noexcept;
void  tc_free(void* ptr) noexcept;
[[nodiscard]] void* tc_calloc(size_t n, size_t size) noexcept;
[[nodiscard]] void* tc_realloc(void* ptr, size_t size) noexcept;
[[nodiscard]] void* tc_memalign(size_t alignment, size_t size) noexcept;
[[nodiscard]] size_t tc_usable_size(void* ptr) noexcept;
void tc_init_allocator() noexcept;

} // namespace tcmalloc
} // namespace my_ptmalloc
