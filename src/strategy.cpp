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
    setenv("MY_MALLOC_MODE", "hybrid", 0);
    my_malloc_init();
}

void ptmalloc_init() noexcept {
    setenv("MY_MALLOC_MODE", "ptmalloc", 0);
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
