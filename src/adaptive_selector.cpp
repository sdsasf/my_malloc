// Runtime telemetry selector for adaptive soft switching.

#include "my_ptmalloc/adaptive_selector.h"
#include "my_ptmalloc/adaptive_mode.h"
#include "my_ptmalloc/adaptive_runtime.h"
#include "my_ptmalloc/adaptive_telemetry.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace my_ptmalloc {

namespace {

std::atomic<uint64_t> g_selector_event_seq{0};
AdaptiveSelectorEvent g_selector_events[ADAPTIVE_SELECTOR_EVENT_RING_SIZE]{};

static bool streq(const char* a, const char* b) noexcept {
    return a && std::strcmp(a, b) == 0;
}

static bool debug_forced() noexcept {
    const char* debug = std::getenv("MY_MALLOC_ADAPTIVE_DEBUG_MODE");
    return streq(debug, "1") || streq(debug, "true");
}

static const char* selector_reason_for_candidate(AdaptiveModeId mode,
                                                 const WorkloadFeatures& f) noexcept {
    if (debug_forced() || f.safety_error_rate > 0.01) return "safety";
    switch (mode) {
        case AdaptiveModeId::CrossThreadMessage:
            return "remote_free_ratio";
        case AdaptiveModeId::LargeObjectStreaming:
            return "large_bytes_ratio";
        case AdaptiveModeId::CompactRSS:
            return "mapped_live_ratio";
        case AdaptiveModeId::DeterministicLatency:
            return "slow_path_ratio";
        case AdaptiveModeId::FragmentationStable:
            return "fragmentation";
        case AdaptiveModeId::ThroughputCache:
            return "cache_reuse";
        case AdaptiveModeId::HardenedDebug:
            return "safety";
        case AdaptiveModeId::Balanced:
        default:
            return "balanced_default";
    }
}

static void record_selector_event(AdaptiveModeId previous,
                                  AdaptiveModeId current,
                                  AdaptiveModeId candidate,
                                  bool switched,
                                  const char* reason,
                                  const WorkloadFeatures& features) noexcept {
    uint64_t seq = g_selector_event_seq.fetch_add(1, std::memory_order_relaxed) + 1;
    AdaptiveSelectorEvent event{};
    event.sequence = seq;
    event.previous_mode = previous;
    event.current_mode = current;
    event.candidate_mode = candidate;
    event.switched = switched;
    std::snprintf(event.reason, sizeof(event.reason), "%s", reason ? reason : "unknown");
    event.features = features;
    g_selector_events[(seq - 1) % ADAPTIVE_SELECTOR_EVENT_RING_SIZE] = event;
}

} // namespace

AdaptiveModeId adaptive_select_mode(const WorkloadFeatures& f) noexcept {
    if (debug_forced() || f.safety_error_rate > 0.01) {
        return AdaptiveModeId::HardenedDebug;
    }
    if (f.remote_free_ratio > 0.20 &&
        (f.remote_free_count + f.same_thread_free_count) >= 64) {
        return AdaptiveModeId::CrossThreadMessage;
    }
    if (f.large_bytes_ratio > 0.50 && f.requested_bytes > 1024 * 1024) {
        return AdaptiveModeId::LargeObjectStreaming;
    }
    if (f.mapped_live_ratio > 8.0 && f.mapped_bytes > 4 * 1024 * 1024) {
        return AdaptiveModeId::CompactRSS;
    }
    if (f.slow_path_ratio > 0.35 && f.alloc_calls > 256) {
        return AdaptiveModeId::DeterministicLatency;
    }
    if (f.size_entropy > 2.2 && f.internal_frag_ratio > 0.20) {
        return AdaptiveModeId::FragmentationStable;
    }
    if (f.cache_hit_rate > 0.70 && f.remote_free_ratio < 0.05 &&
        f.mapped_live_ratio < 6.0 && f.alloc_calls > 512) {
        return AdaptiveModeId::ThroughputCache;
    }
    return AdaptiveModeId::Balanced;
}

void adaptive_selector_maybe_switch() noexcept {
    adaptive_runtime_init();
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    if (cfg.selector == AdaptiveModeSelectorKind::Fixed ||
        cfg.selector == AdaptiveModeSelectorKind::Manual) {
        return;
    }

    uint64_t n = adaptive_selector_window_counter();
    if ((n % cfg.mode_window) != 0) {
        return;
    }

    WorkloadFeatures f = adaptive_extract_window_features();
    AdaptiveModeId current = adaptive_current_mode();
    AdaptiveModeId next = adaptive_select_mode(f);
    AdaptiveModeId previous = adaptive_previous_mode();
    const char* reason = selector_reason_for_candidate(next, f);
    if (n < adaptive_selector_cooldown_until()) {
        record_selector_event(previous, current, next, false, "cooldown", f);
        return;
    }
    if (next == current) {
        record_selector_event(previous, current, next, false, reason, f);
        return;
    }

    bool severe_memory_pressure = f.mapped_live_ratio > 12.0 && f.mapped_bytes > 8 * 1024 * 1024;
    bool expected_gain = severe_memory_pressure ||
        next == AdaptiveModeId::HardenedDebug ||
        f.remote_free_ratio > 0.25 ||
        f.large_bytes_ratio > 0.60 ||
        f.slow_path_ratio > 0.45 ||
        f.internal_frag_ratio > 0.30 ||
        f.cache_hit_rate > 0.80;
    if (!expected_gain) {
        record_selector_event(previous, current, next, false, "hysteresis", f);
        return;
    }

    adaptive_activate_mode(next);
    adaptive_selector_set_cooldown_until(
        n + static_cast<uint64_t>(cfg.mode_cooldown) * static_cast<uint64_t>(cfg.mode_window));
    adaptive_mode_policy(next).on_window();
    record_selector_event(previous, current, next, true, reason, f);
}

bool adaptive_selector_last_event(AdaptiveSelectorEvent& out) noexcept {
    uint64_t seq = g_selector_event_seq.load(std::memory_order_relaxed);
    if (seq == 0) return false;
    out = g_selector_events[(seq - 1) % ADAPTIVE_SELECTOR_EVENT_RING_SIZE];
    return out.sequence != 0;
}

size_t adaptive_selector_events_snapshot(AdaptiveSelectorEvent* out, size_t max_events) noexcept {
    if (!out || max_events == 0) return 0;
    uint64_t seq = g_selector_event_seq.load(std::memory_order_relaxed);
    if (seq == 0) return 0;
    size_t available = static_cast<size_t>(
        seq < ADAPTIVE_SELECTOR_EVENT_RING_SIZE ? seq : ADAPTIVE_SELECTOR_EVENT_RING_SIZE);
    size_t count = available < max_events ? available : max_events;
    uint64_t first = seq - count + 1;
    for (size_t i = 0; i < count; ++i) {
        uint64_t event_seq = first + i;
        out[i] = g_selector_events[(event_seq - 1) % ADAPTIVE_SELECTOR_EVENT_RING_SIZE];
    }
    return count;
}

} // namespace my_ptmalloc
