#pragma once
// mimalloc public API — Microsoft-style memory allocator.

#include "types.h"

namespace my_ptmalloc {
namespace mimalloc {

// ─── Core API ───
[[nodiscard]] void* mi_malloc(size_t size) noexcept;
void  mi_free(void* ptr) noexcept;
[[nodiscard]] void* mi_calloc(size_t n, size_t size) noexcept;
[[nodiscard]] void* mi_realloc(void* ptr, size_t size) noexcept;

// ─── Aligned API ───
[[nodiscard]] void* mi_memalign(size_t alignment, size_t size) noexcept;
[[nodiscard]] size_t mi_usable_size(void* ptr) noexcept;

// ─── Init ───
void mi_init() noexcept;

} // namespace mimalloc
} // namespace my_ptmalloc
