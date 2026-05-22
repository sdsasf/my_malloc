// Aggregated registry: maps strategy names to classic allocator descriptors.

#include "classic/common/strategy_registry.h"

namespace my_ptmalloc {

// Each classic allocator's strategy descriptor is defined in its own
// *_strategy.cpp file. This registry just re-exports them.

// Actual implementations live in:
//   classic/ptmalloc/ptmalloc_strategy.cpp
//   classic/tcmalloc/tcmalloc_strategy.cpp
//   classic/jemalloc/jemalloc_strategy.cpp
//   classic/mimalloc/mimalloc_strategy.cpp

} // namespace my_ptmalloc
