// Strategy descriptor for tcmalloc.

#include "my_ptmalloc/strategy.h"
#include "tcmalloc.h"

namespace my_ptmalloc {

StrategyDescriptor tcmalloc_classic_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "tcmalloc",
        "Google tcmalloc: 86 real size classes, ThreadCache(slow-start GC) → "
        "CentralFreeList(batch transfer, spinlock) → PageHeap(span free lists), "
        "2-level PageMap radix tree (page→Span O(1)), Span refcount tracking, "
        "aggressive decommit",
        StrategyVTable{
            tcmalloc::tc_init_allocator,
            []() noexcept {},
            tcmalloc::tc_malloc,
            tcmalloc::tc_free,
            tcmalloc::tc_realloc,
            tcmalloc::tc_usable_size,
            []() noexcept { return StrategyStats{}; },
        },
    };
}

} // namespace my_ptmalloc
