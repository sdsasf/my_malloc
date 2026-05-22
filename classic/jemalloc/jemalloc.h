#pragma once
// jemalloc public API.

#include "size_classes.h"

namespace my_ptmalloc {
namespace jemalloc {

[[nodiscard]] void* je_malloc(size_t size) noexcept;
void  je_free(void* ptr) noexcept;
[[nodiscard]] void* je_calloc(size_t n, size_t size) noexcept;
[[nodiscard]] void* je_realloc(void* ptr, size_t size) noexcept;
[[nodiscard]] void* je_memalign(size_t alignment, size_t size) noexcept;
[[nodiscard]] size_t je_usable_size(void* ptr) noexcept;
void je_init() noexcept;

} // namespace jemalloc
} // namespace my_ptmalloc
