#pragma once
// Runtime allocator selection layer.

#include <cstddef>

namespace my_ptmalloc {

enum class RuntimeAllocatorKind {
    Hybrid,
    Ptmalloc,
    TcmallocLike,
    JemallocLike,
    MimallocLike,
};

[[nodiscard]] RuntimeAllocatorKind runtime_select_allocator(size_t size) noexcept;
[[nodiscard]] const char* runtime_allocator_name(RuntimeAllocatorKind kind) noexcept;
[[nodiscard]] bool runtime_mode_uses_family_allocators() noexcept;
[[nodiscard]] bool runtime_mode_is_adaptive() noexcept;

} // namespace my_ptmalloc
