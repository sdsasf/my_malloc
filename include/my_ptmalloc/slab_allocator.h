#pragma once
// Small-object slab allocator.
// This is a non-ptmalloc fast path for <= 1024 byte normal allocations.

#include <cstddef>

namespace my_ptmalloc {

constexpr size_t SLAB_MAX_ALLOC = 1024;

[[nodiscard]] void* slab_malloc(size_t size) noexcept;
[[nodiscard]] bool slab_free(void* ptr) noexcept;
[[nodiscard]] bool slab_contains(void* ptr) noexcept;
[[nodiscard]] size_t slab_usable_size(void* ptr) noexcept;

class ScopedSlabBypass {
    bool old_;
public:
    ScopedSlabBypass() noexcept;
    ~ScopedSlabBypass() noexcept;
    ScopedSlabBypass(const ScopedSlabBypass&) = delete;
    ScopedSlabBypass& operator=(const ScopedSlabBypass&) = delete;
};

} // namespace my_ptmalloc
