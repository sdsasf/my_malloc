#pragma once
// Internal adaptive runtime state: env config, active mode, mode states, and
// soft-switch transitions. This layer contains no allocation implementation.

#include "my_ptmalloc/adaptive_types.h"

namespace my_ptmalloc {

enum class AdaptiveModeSelectorKind : uint8_t {
    Rule = 0,
    Fixed = 1,
    Manual = 2,
    Model = 3,
};

enum class AdaptiveModeState : uint8_t {
    Inactive = 0,
    Active = 1,
    Retired = 2,
};

struct AdaptiveRuntimeConfig {
    uint32_t version;
    size_t small_page_size;
    size_t medium_span_size;
    uint32_t empty_cache_limit;
    AdaptiveModeId configured_mode;
    AdaptiveModeSelectorKind selector;
    uint32_t mode_window;
    uint32_t mode_cooldown;
};

void adaptive_runtime_init() noexcept;
AdaptiveRuntimeConfig adaptive_runtime_config() noexcept;
AdaptiveModeId adaptive_current_mode() noexcept;
AdaptiveModeId adaptive_previous_mode() noexcept;
AdaptiveModeSelectorKind adaptive_selector_kind() noexcept;
uint64_t adaptive_selector_window_counter() noexcept;
uint64_t adaptive_selector_cooldown_until() noexcept;
void adaptive_selector_set_cooldown_until(uint64_t value) noexcept;
void adaptive_activate_mode(AdaptiveModeId mode) noexcept;
void adaptive_set_mode(AdaptiveModeId mode) noexcept;
bool adaptive_valid_mode_id(AdaptiveModeId mode) noexcept;
size_t adaptive_mode_index(AdaptiveModeId mode) noexcept;
uint64_t adaptive_thread_token() noexcept;
uint64_t adaptive_next_alloc_epoch() noexcept;
uint64_t adaptive_retired_mode_count() noexcept;
AdaptiveModeState adaptive_mode_state(AdaptiveModeId mode) noexcept;

} // namespace my_ptmalloc
