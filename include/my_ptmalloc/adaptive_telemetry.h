#pragma once
// Runtime telemetry for the adaptive allocator. Hot-path functions keep to
// relaxed counters; richer feature extraction is done at selector boundaries.

#include "my_ptmalloc/adaptive_services.h"

namespace my_ptmalloc {

void adaptive_telemetry_on_malloc_call() noexcept;
void adaptive_telemetry_on_alloc(const AllocationResult& result) noexcept;
void adaptive_telemetry_on_alloc_failure() noexcept;
void adaptive_telemetry_on_free_call() noexcept;
void adaptive_telemetry_on_free_begin(AdaptiveHeader* hdr) noexcept;
void adaptive_telemetry_on_free_end(AdaptiveHeader* hdr) noexcept;
void adaptive_telemetry_on_invalid_free(void* ptr) noexcept;
void adaptive_telemetry_on_realloc() noexcept;
void adaptive_telemetry_on_mapped(AdaptiveModeId mode, size_t bytes) noexcept;
void adaptive_telemetry_on_unmapped(AdaptiveModeId mode, size_t bytes) noexcept;
void adaptive_telemetry_on_pool_hit(AdaptiveStorageId storage) noexcept;
void adaptive_telemetry_on_pool_miss(AdaptiveStorageId storage) noexcept;
void adaptive_telemetry_on_empty_counts(AdaptiveStorageId storage, uint32_t count) noexcept;
void adaptive_telemetry_on_release(AdaptiveStorageId storage, size_t bytes) noexcept;
void adaptive_telemetry_on_mmap() noexcept;
void adaptive_telemetry_on_munmap() noexcept;
void adaptive_telemetry_on_header_corruption() noexcept;
void adaptive_telemetry_on_double_free() noexcept;
void adaptive_telemetry_on_mode_switch() noexcept;

AdaptiveStatsSnapshot adaptive_stats_snapshot() noexcept;
void adaptive_stats_reset() noexcept;
WorkloadFeatures adaptive_extract_window_features() noexcept;

} // namespace my_ptmalloc
