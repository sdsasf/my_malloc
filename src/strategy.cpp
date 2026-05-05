// Built-in strategy descriptors.

#include "my_ptmalloc/strategy.h"
#include "my_ptmalloc/my_malloc.h"

#include <cstdlib>

namespace my_ptmalloc {

namespace {

StrategyStats empty_stats() noexcept {
    return StrategyStats{0, 0, 0, 0};
}

void noop_init() noexcept {}
void noop_shutdown() noexcept {}

void hybrid_init() noexcept {
    setenv("MY_MALLOC_MODE", "hybrid", 1);
    my_malloc_init();
}

void ptmalloc_init() noexcept {
    setenv("MY_MALLOC_MODE", "ptmalloc", 1);
    my_malloc_init();
}

void tcmalloc_like_init() noexcept {
    setenv("MY_MALLOC_MODE", "tcmalloc_like", 1);
    my_malloc_init();
}

void jemalloc_like_init() noexcept {
    setenv("MY_MALLOC_MODE", "jemalloc_like", 1);
    my_malloc_init();
}

void mimalloc_like_init() noexcept {
    setenv("MY_MALLOC_MODE", "mimalloc_like", 1);
    my_malloc_init();
}

void adaptive_init() noexcept {
    setenv("MY_MALLOC_MODE", "adaptive", 1);
    my_malloc_init();
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

} // namespace

StrategyDescriptor hybrid_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "hybrid",
        "Small-object slab frontend with ptmalloc-style fallback",
        StrategyVTable{
            hybrid_init,
            noop_shutdown,
            my_malloc,
            my_free,
            my_realloc,
            my_malloc_usable_size,
            []() noexcept { return StrategyStats{0, 0, 0, 0}; },
        },
    };
}

StrategyDescriptor ptmalloc_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "ptmalloc",
        "Chunk/bin/arena backend with slab frontend disabled",
        StrategyVTable{
            ptmalloc_init,
            noop_shutdown,
            my_malloc,
            my_free,
            my_realloc,
            my_malloc_usable_size,
            []() noexcept { return StrategyStats{0, 0, 0, 0}; },
        },
    };
}

StrategyDescriptor tcmalloc_like_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "tcmalloc_like",
        "Teaching size-class allocator with thread caches, central free lists, and 64KB spans",
        StrategyVTable{
            tcmalloc_like_init,
            noop_shutdown,
            my_malloc,
            my_free,
            my_realloc,
            my_malloc_usable_size,
            []() noexcept { return StrategyStats{0, 0, 0, 0}; },
        },
    };
}

StrategyDescriptor jemalloc_like_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "jemalloc_like",
        "Teaching arena/run allocator with per-thread tcache and arena-local non-full runs",
        StrategyVTable{
            jemalloc_like_init,
            noop_shutdown,
            my_malloc,
            my_free,
            my_realloc,
            my_malloc_usable_size,
            []() noexcept { return StrategyStats{0, 0, 0, 0}; },
        },
    };
}

StrategyDescriptor mimalloc_like_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "mimalloc_like",
        "Teaching per-thread heap/page allocator with owner remote-free queues",
        StrategyVTable{
            mimalloc_like_init,
            noop_shutdown,
            my_malloc,
            my_free,
            my_realloc,
            my_malloc_usable_size,
            []() noexcept { return StrategyStats{0, 0, 0, 0}; },
        },
    };
}

StrategyDescriptor adaptive_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "adaptive",
        "Adaptive selection policy over concrete teaching allocator implementations",
        StrategyVTable{
            adaptive_init,
            noop_shutdown,
            my_malloc,
            my_free,
            my_realloc,
            my_malloc_usable_size,
            []() noexcept { return StrategyStats{0, 0, 0, 0}; },
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
