#pragma once
// Aggregated strategy descriptors from all classic allocator reproductions.

#include "my_ptmalloc/strategy.h"

namespace my_ptmalloc {

// Each classic allocator exports its own descriptor function.
// These replace the old mixed dispatch versions.
StrategyDescriptor ptmalloc_classic_strategy_descriptor() noexcept;
StrategyDescriptor tcmalloc_classic_strategy_descriptor() noexcept;
StrategyDescriptor jemalloc_classic_strategy_descriptor() noexcept;
StrategyDescriptor mimalloc_classic_strategy_descriptor() noexcept;

} // namespace my_ptmalloc
