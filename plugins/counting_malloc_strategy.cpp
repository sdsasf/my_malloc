// Example external strategy plugin.
// Build target: example_counting_strategy

#include "my_ptmalloc/strategy.h"

#include <atomic>
#include <cstdlib>
#include <malloc.h>

namespace {

std::atomic<uint64_t> alloc_calls{0};
std::atomic<uint64_t> free_calls{0};
std::atomic<uint64_t> realloc_calls{0};
std::atomic<uint64_t> usable_calls{0};

void init() noexcept {}
void shutdown() noexcept {}

void* allocate(size_t size) noexcept {
    alloc_calls.fetch_add(1, std::memory_order_relaxed);
    return std::malloc(size);
}

void deallocate(void* ptr) noexcept {
    free_calls.fetch_add(1, std::memory_order_relaxed);
    std::free(ptr);
}

void* reallocate(void* ptr, size_t size) noexcept {
    realloc_calls.fetch_add(1, std::memory_order_relaxed);
    return std::realloc(ptr, size);
}

size_t usable_size(void* ptr) noexcept {
    usable_calls.fetch_add(1, std::memory_order_relaxed);
    return ptr ? malloc_usable_size(ptr) : 0;
}

my_ptmalloc::StrategyStats stats() noexcept {
    return my_ptmalloc::StrategyStats{
        alloc_calls.load(std::memory_order_relaxed),
        free_calls.load(std::memory_order_relaxed),
        realloc_calls.load(std::memory_order_relaxed),
        usable_calls.load(std::memory_order_relaxed),
    };
}

} // namespace

extern "C" my_ptmalloc::StrategyDescriptor my_malloc_get_strategy() noexcept {
    return my_ptmalloc::StrategyDescriptor{
        my_ptmalloc::STRATEGY_API_VERSION,
        "counting_malloc",
        "Example plugin wrapping libc malloc with simple counters",
        my_ptmalloc::StrategyVTable{
            init,
            shutdown,
            allocate,
            deallocate,
            reallocate,
            usable_size,
            stats,
        },
    };
}
