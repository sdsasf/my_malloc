// Strategy descriptor for mimalloc.

#include "my_ptmalloc/strategy.h"
#include "mimalloc.h"

namespace my_ptmalloc {

StrategyDescriptor mimalloc_classic_strategy_descriptor() noexcept {
    return StrategyDescriptor{
        STRATEGY_API_VERSION,
        "mimalloc",
        "Microsoft mimalloc: per-thread independent heaps, pointer-arithmetic "
        "metadata lookup (no hash table), atomic remote-free queues, "
        "abandoned page reclaim, segment cache, free list obfuscation",
        StrategyVTable{
            mimalloc::mi_init,
            []() noexcept {},
            mimalloc::mi_malloc,
            mimalloc::mi_free,
            mimalloc::mi_realloc,
            mimalloc::mi_usable_size,
            []() noexcept { return StrategyStats{}; },
        },
    };
}

} // namespace my_ptmalloc
