// Runtime telemetry selector for adaptive soft switching.

#include "my_ptmalloc/adaptive_selector.h"
#include "my_ptmalloc/adaptive_mode.h"
#include "my_ptmalloc/adaptive_runtime.h"
#include "my_ptmalloc/adaptive_telemetry.h"

#include <cstdlib>
#include <cstring>

namespace my_ptmalloc {

namespace {

static bool streq(const char* a, const char* b) noexcept {
    return a && std::strcmp(a, b) == 0;
}

static bool debug_forced() noexcept {
    const char* debug = std::getenv("MY_MALLOC_ADAPTIVE_DEBUG_MODE");
    return streq(debug, "1") || streq(debug, "true");
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
    if (n < adaptive_selector_cooldown_until() || (n % cfg.mode_window) != 0) {
        return;
    }

    WorkloadFeatures f = adaptive_extract_window_features();
    AdaptiveModeId current = adaptive_current_mode();
    AdaptiveModeId next = adaptive_select_mode(f);
    if (next == current) return;

    bool severe_memory_pressure = f.mapped_live_ratio > 12.0 && f.mapped_bytes > 8 * 1024 * 1024;
    bool expected_gain = severe_memory_pressure ||
        next == AdaptiveModeId::HardenedDebug ||
        f.remote_free_ratio > 0.25 ||
        f.large_bytes_ratio > 0.60 ||
        f.slow_path_ratio > 0.45 ||
        f.internal_frag_ratio > 0.30 ||
        f.cache_hit_rate > 0.80;
    if (!expected_gain) return;

    adaptive_activate_mode(next);
    adaptive_selector_set_cooldown_until(
        n + static_cast<uint64_t>(cfg.mode_cooldown) * static_cast<uint64_t>(cfg.mode_window));
    adaptive_mode_policy(next).on_window();
}

} // namespace my_ptmalloc
