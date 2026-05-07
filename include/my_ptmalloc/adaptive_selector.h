#pragma once
// Runtime mode selector and soft switching.

#include "my_ptmalloc/adaptive_types.h"

namespace my_ptmalloc {

void adaptive_selector_maybe_switch() noexcept;
AdaptiveModeId adaptive_select_mode(const WorkloadFeatures& f) noexcept;

} // namespace my_ptmalloc
