#pragma once
// Offline-trained selector model interface. The online path evaluates a tiny
// generated tree ensemble at selector-window boundaries only.

#include "my_ptmalloc/adaptive_types.h"

namespace my_ptmalloc {

struct AdaptiveModelDecision {
    AdaptiveModeId mode;
    double cost[ADAPTIVE_MODE_COUNT];
    double confidence;
    bool available;
};

AdaptiveModelDecision adaptive_model_select_mode(const WorkloadFeatures& f,
                                                 AdaptiveModeId current) noexcept;
const char* adaptive_model_id() noexcept;

} // namespace my_ptmalloc
