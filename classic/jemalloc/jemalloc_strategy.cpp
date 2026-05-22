// Strategy descriptor for jemalloc.

#include "my_ptmalloc/strategy.h"
#include "jemalloc.h"

namespace my_ptmalloc {

StrategyDescriptor jemalloc_classic_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "jemalloc",
        "FreeBSD jemalloc: 36 real size classes, multi-arena(round-robin/affinity), "
        "per-thread tcache → arena bin → run hierarchy, run bitmap, "
        "extent buddy allocator (dirty/muzzy/clean), rtree(addr→extent), "
        "huge objects (>2MB direct mmap)",
        StrategyVTable{
            jemalloc::je_init,
            []() noexcept {},
            jemalloc::je_malloc,
            jemalloc::je_free,
            jemalloc::je_realloc,
            jemalloc::je_usable_size,
            []() noexcept { return StrategyStats{}; },
        },
    };
}

} // namespace my_ptmalloc
