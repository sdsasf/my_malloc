// Built-in strategy descriptors: adaptive and libc baseline.

#include "my_ptmalloc/strategy.h"
#include "my_ptmalloc/adaptive_allocator.h"

#include <cstdlib>

namespace my_ptmalloc {

namespace {

StrategyStats empty_stats() noexcept {
    return StrategyStats{0, 0, 0, 0};
}

void noop_init() noexcept {}
void noop_shutdown() noexcept {}

void adaptive_init() noexcept {
    // Adaptive allocator initializes on first use via its own init path.
}

void* libc_malloc_wrap(size_t size) noexcept {
    return std::malloc(size);
}

void libc_free_wrap(void* ptr) noexcept {
    std::free(ptr);
}

void* libc_realloc_wrap(void* ptr, size_t size) noexcept {
    return std::realloc(ptr, size);
}

size_t libc_usable_size_wrap(void*) noexcept {
    return 0;
}

StrategyStats adaptive_stats_wrap() noexcept {
    AdaptiveStatsSnapshot s = adaptive_stats_snapshot();
    return StrategyStats{
        s.malloc_calls,
        s.free_calls,
        s.realloc_calls,
        0,
    };
}

} // namespace

StrategyDescriptor adaptive_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "adaptive",
        "Independent multi-mode adaptive allocator with shared metadata and soft switching",
        StrategyVTable{
            adaptive_init,
            noop_shutdown,
            adaptive_malloc,
            adaptive_free,
            adaptive_realloc,
            adaptive_usable_size,
            adaptive_stats_wrap,
        },
    };
}

StrategyDescriptor libc_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "libc",
        "System libc malloc/free/realloc baseline",
        StrategyVTable{
            noop_init,
            noop_shutdown,
            libc_malloc_wrap,
            libc_free_wrap,
            libc_realloc_wrap,
            libc_usable_size_wrap,
            empty_stats,
        },
    };
}

} // namespace my_ptmalloc
