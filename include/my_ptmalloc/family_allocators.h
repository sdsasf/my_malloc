#pragma once
// Teaching allocator-family implementations used by runtime modes.

#include <cstddef>

namespace my_ptmalloc {

[[nodiscard]] void* tcmalloc_like_malloc(size_t size) noexcept;
[[nodiscard]] void* jemalloc_like_malloc(size_t size) noexcept;
[[nodiscard]] void* mimalloc_like_malloc(size_t size) noexcept;

[[nodiscard]] bool family_free(void* ptr) noexcept;
[[nodiscard]] void* family_realloc(void* ptr, size_t size) noexcept;
[[nodiscard]] size_t family_usable_size(void* ptr) noexcept;
[[nodiscard]] void* family_memalign(size_t alignment, size_t size) noexcept;

} // namespace my_ptmalloc
