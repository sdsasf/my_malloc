#pragma once
// Public facade for the independent adaptive allocator backend.
//
// The implementation is split into:
// - shared memory management services;
// - adaptive mode policy;
// - runtime telemetry and selector.

#include "my_ptmalloc/adaptive_types.h"

namespace my_ptmalloc {

// --- Public API ------------------------------------------------------------

[[nodiscard]] void*  adaptive_malloc(size_t size) noexcept;
void                 adaptive_free(void* ptr) noexcept;
[[nodiscard]] void*  adaptive_realloc(void* ptr, size_t size) noexcept;
[[nodiscard]] size_t adaptive_usable_size(void* ptr) noexcept;
[[nodiscard]] void*  adaptive_memalign(size_t alignment, size_t size) noexcept;

// Ownership query: is this pointer owned by the adaptive allocator?
[[nodiscard]] bool   adaptive_owns(void* ptr) noexcept;

// Stats
[[nodiscard]] AdaptiveStatsSnapshot adaptive_stats_snapshot() noexcept;
void adaptive_stats_reset() noexcept;

[[nodiscard]] AdaptiveModeId adaptive_current_mode() noexcept;
[[nodiscard]] const char* adaptive_mode_name(AdaptiveModeId m) noexcept;
[[nodiscard]] const char* adaptive_mode_objective(AdaptiveModeId m) noexcept;
void adaptive_set_mode(AdaptiveModeId mode) noexcept;

} // namespace my_ptmalloc
