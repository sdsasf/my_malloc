#pragma once
// Runtime mode selector and soft switching.

#include "my_ptmalloc/adaptive_types.h"

namespace my_ptmalloc {

static constexpr size_t ADAPTIVE_SELECTOR_EVENT_RING_SIZE = 64;

struct AdaptiveSelectorEvent {
    uint64_t sequence;
    AdaptiveModeId previous_mode;
    AdaptiveModeId current_mode;
    AdaptiveModeId candidate_mode;
    bool switched;
    char reason[32];
    char selector_backend[16];
    AdaptiveModeId rule_candidate;
    AdaptiveModeId model_candidate;
    double model_confidence;
    WorkloadFeatures features;
};

void adaptive_selector_maybe_switch() noexcept;
AdaptiveModeId adaptive_select_mode(const WorkloadFeatures& f) noexcept;
bool adaptive_selector_last_event(AdaptiveSelectorEvent& out) noexcept;
size_t adaptive_selector_events_snapshot(AdaptiveSelectorEvent* out, size_t max_events) noexcept;

} // namespace my_ptmalloc
