// Strategy descriptor for ptmalloc — plugs into the unified benchmark framework.

#include "my_ptmalloc/strategy.h"
#include "ptmalloc.h"

namespace my_ptmalloc {
namespace ptmalloc {

static void pt_init() noexcept {
    pt_malloc_init();
}

static void pt_shutdown() noexcept {
    // No explicit shutdown needed
}

static StrategyStats pt_stats() noexcept {
    StrategyStats s{};
    s.alloc_calls    = 0;
    s.free_calls     = 0;
    s.realloc_calls  = 0;
    s.usable_size_calls = 0;
    return s;
}

} // namespace ptmalloc

StrategyDescriptor ptmalloc_classic_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "ptmalloc",
        "glibc ptmalloc: boundary-tag chunks, fast/small/large/unsorted bins, "
        "tcache (64-bin/7-entry), multi-arena, HeapInfo, coalescing, mmap threshold",
        StrategyVTable{
            ptmalloc::pt_init,
            ptmalloc::pt_shutdown,
            ptmalloc::pt_malloc,
            ptmalloc::pt_free,
            ptmalloc::pt_realloc,
            ptmalloc::pt_malloc_usable_size,
            ptmalloc::pt_stats,
        },
    };
}

} // namespace my_ptmalloc
