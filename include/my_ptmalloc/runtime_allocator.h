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

enum class AdaptivePolicyKind {
    Heuristic,
    RoundRobin,
    Bandit,
    Fixed,
};

[[nodiscard]] RuntimeAllocatorKind runtime_select_allocator(size_t size) noexcept;
[[nodiscard]] const char* runtime_allocator_name(RuntimeAllocatorKind kind) noexcept;
[[nodiscard]] AdaptivePolicyKind runtime_adaptive_policy() noexcept;
[[nodiscard]] const char* runtime_adaptive_policy_name() noexcept;
[[nodiscard]] bool runtime_mode_uses_family_allocators() noexcept;
[[nodiscard]] bool runtime_mode_is_adaptive() noexcept;
[[nodiscard]] bool runtime_mode_is_adaptive_demo() noexcept;

} // namespace my_ptmalloc
