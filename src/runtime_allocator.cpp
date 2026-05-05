// Runtime allocator selection layer.

#include "my_ptmalloc/runtime_allocator.h"
#include "my_ptmalloc/allocator_lab.h"

#include <atomic>
#include <cstdlib>
#include <cstring>

namespace my_ptmalloc {

namespace {

constexpr size_t KIND_COUNT = 5;
std::atomic<uint64_t> rr_counter{0};
std::atomic<uint64_t> bandit_counts[KIND_COUNT]{};
bool policy_initialized = false;
AdaptivePolicyKind policy_kind = AdaptivePolicyKind::Heuristic;
RuntimeAllocatorKind fixed_kind = RuntimeAllocatorKind::Hybrid;

[[nodiscard]] size_t kind_index(RuntimeAllocatorKind kind) noexcept {
    return static_cast<size_t>(kind);
}

[[nodiscard]] RuntimeAllocatorKind index_kind(size_t idx) noexcept {
    switch (idx % KIND_COUNT) {
        case 0: return RuntimeAllocatorKind::Hybrid;
        case 1: return RuntimeAllocatorKind::Ptmalloc;
        case 2: return RuntimeAllocatorKind::TcmallocLike;
        case 3: return RuntimeAllocatorKind::JemallocLike;
        case 4: return RuntimeAllocatorKind::MimallocLike;
    }
    return RuntimeAllocatorKind::Hybrid;
}

[[nodiscard]] bool streq(const char* a, const char* b) noexcept {
    return a && std::strcmp(a, b) == 0;
}

[[nodiscard]] RuntimeAllocatorKind parse_allocator_kind(const char* name,
                                                        RuntimeAllocatorKind fallback) noexcept {
    if (streq(name, "hybrid")) return RuntimeAllocatorKind::Hybrid;
    if (streq(name, "ptmalloc")) return RuntimeAllocatorKind::Ptmalloc;
    if (streq(name, "tcmalloc") || streq(name, "tcmalloc_like")) return RuntimeAllocatorKind::TcmallocLike;
    if (streq(name, "jemalloc") || streq(name, "jemalloc_like")) return RuntimeAllocatorKind::JemallocLike;
    if (streq(name, "mimalloc") || streq(name, "mimalloc_like")) return RuntimeAllocatorKind::MimallocLike;
    return fallback;
}

void init_policy() noexcept {
    if (policy_initialized) return;
    policy_initialized = true;

    const char* policy = std::getenv("MY_MALLOC_ADAPTIVE_POLICY");
    if (!policy || streq(policy, "heuristic")) {
        policy_kind = AdaptivePolicyKind::Heuristic;
        return;
    }
    if (streq(policy, "round_robin")) {
        policy_kind = AdaptivePolicyKind::RoundRobin;
        return;
    }
    if (streq(policy, "bandit") || streq(policy, "rl_bandit")) {
        policy_kind = AdaptivePolicyKind::Bandit;
        return;
    }
    constexpr const char* fixed_prefix = "fixed:";
    constexpr size_t fixed_prefix_len = 6;
    if (std::strncmp(policy, fixed_prefix, fixed_prefix_len) == 0) {
        policy_kind = AdaptivePolicyKind::Fixed;
        fixed_kind = parse_allocator_kind(policy + fixed_prefix_len, RuntimeAllocatorKind::Hybrid);
        return;
    }
    policy_kind = AdaptivePolicyKind::Heuristic;
}

[[nodiscard]] RuntimeAllocatorKind heuristic_select(size_t size) noexcept {
    if (size <= 128) return RuntimeAllocatorKind::MimallocLike;
    if (size <= 1024) return RuntimeAllocatorKind::TcmallocLike;
    if (size <= 4096) return RuntimeAllocatorKind::JemallocLike;
    if (size <= 64 * 1024) return RuntimeAllocatorKind::Ptmalloc;
    return RuntimeAllocatorKind::Hybrid;
}

[[nodiscard]] RuntimeAllocatorKind round_robin_select() noexcept {
    uint64_t next = rr_counter.fetch_add(1, std::memory_order_relaxed);
    return index_kind(static_cast<size_t>(next));
}

[[nodiscard]] RuntimeAllocatorKind bandit_select(size_t size) noexcept {
    RuntimeAllocatorKind preferred = heuristic_select(size);
    size_t preferred_idx = kind_index(preferred);
    uint64_t preferred_count = bandit_counts[preferred_idx].load(std::memory_order_relaxed);

    for (size_t i = 0; i < KIND_COUNT; ++i) {
        uint64_t count = bandit_counts[i].load(std::memory_order_relaxed);
        if (count + 4 < preferred_count) {
            bandit_counts[i].fetch_add(1, std::memory_order_relaxed);
            return index_kind(i);
        }
    }

    bandit_counts[preferred_idx].fetch_add(1, std::memory_order_relaxed);
    return preferred;
}

} // namespace

RuntimeAllocatorKind runtime_select_allocator(size_t size) noexcept {
    switch (allocator_mode()) {
        case AllocMode::Hybrid:
            return RuntimeAllocatorKind::Hybrid;
        case AllocMode::PtmallocOnly:
            return RuntimeAllocatorKind::Ptmalloc;
        case AllocMode::TcmallocLike:
            return RuntimeAllocatorKind::TcmallocLike;
        case AllocMode::JemallocLike:
            return RuntimeAllocatorKind::JemallocLike;
        case AllocMode::MimallocLike:
            return RuntimeAllocatorKind::MimallocLike;
        case AllocMode::Adaptive:
            // Independent adaptive allocator — handled directly in
            // malloc_impl/free_impl/realloc_impl, never reaches here.
            return RuntimeAllocatorKind::Hybrid;
        case AllocMode::AdaptiveDemo:
            // Legacy demo: dispatch across teaching allocators
            init_policy();
            switch (policy_kind) {
                case AdaptivePolicyKind::Heuristic:
                    return heuristic_select(size);
                case AdaptivePolicyKind::RoundRobin:
                    return round_robin_select();
                case AdaptivePolicyKind::Bandit:
                    return bandit_select(size);
                case AdaptivePolicyKind::Fixed:
                    return fixed_kind;
            }
    }
    return RuntimeAllocatorKind::Hybrid;
}

const char* runtime_allocator_name(RuntimeAllocatorKind kind) noexcept {
    switch (kind) {
        case RuntimeAllocatorKind::Hybrid:
            return "hybrid";
        case RuntimeAllocatorKind::Ptmalloc:
            return "ptmalloc";
        case RuntimeAllocatorKind::TcmallocLike:
            return "tcmalloc_like";
        case RuntimeAllocatorKind::JemallocLike:
            return "jemalloc_like";
        case RuntimeAllocatorKind::MimallocLike:
            return "mimalloc_like";
    }
    return "unknown";
}

AdaptivePolicyKind runtime_adaptive_policy() noexcept {
    init_policy();
    return policy_kind;
}

const char* runtime_adaptive_policy_name() noexcept {
    switch (runtime_adaptive_policy()) {
        case AdaptivePolicyKind::Heuristic:
            return "heuristic";
        case AdaptivePolicyKind::RoundRobin:
            return "round_robin";
        case AdaptivePolicyKind::Bandit:
            return "bandit";
        case AdaptivePolicyKind::Fixed:
            return "fixed";
    }
    return "unknown";
}

bool runtime_mode_uses_family_allocators() noexcept {
    switch (allocator_mode()) {
        case AllocMode::TcmallocLike:
        case AllocMode::JemallocLike:
        case AllocMode::MimallocLike:
        case AllocMode::AdaptiveDemo:
            return true;
        case AllocMode::Hybrid:
        case AllocMode::PtmallocOnly:
        case AllocMode::Adaptive:
            return false;
    }
    return false;
}

bool runtime_mode_is_adaptive() noexcept {
    return allocator_mode() == AllocMode::Adaptive;
}

bool runtime_mode_is_adaptive_demo() noexcept {
    return allocator_mode() == AllocMode::AdaptiveDemo;
}

} // namespace my_ptmalloc
