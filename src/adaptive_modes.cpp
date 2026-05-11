// Adaptive Mode Policy Layer implementation.

#include "my_ptmalloc/adaptive_mode.h"
#include "my_ptmalloc/adaptive_runtime.h"

namespace my_ptmalloc {

namespace {

static uint32_t configured_empty_keep() noexcept {
    return adaptive_runtime_config().empty_cache_limit;
}

static void noop() noexcept {}

static AllocationPlan make_plan(StoragePreference storage,
                                bool thread_cache,
                                bool reuse,
                                bool low_rss,
                                bool redzone,
                                bool quarantine,
                                bool direct,
                                bool drain_remote,
                                bool occupancy_pack,
                                uint32_t batch,
                                uint32_t keep,
                                uint32_t tcache_small,
                                uint32_t tcache_medium,
                                size_t direct_threshold) noexcept {
    return AllocationPlan{storage, thread_cache, reuse, low_rss, redzone,
                          quarantine, direct, drain_remote, occupancy_pack,
                          batch, keep, tcache_small, tcache_medium,
                          direct_threshold};
}

static ReleaseDecision make_release(ReleaseAction action,
                                    bool poison = false,
                                    bool redzone = false,
                                    bool quarantine = false,
                                    bool remote_queue = false,
                                    uint32_t keep = 0,
                                    uint32_t tcache_small = 0,
                                    uint32_t tcache_medium = 0) noexcept {
    return ReleaseDecision{action, poison, redzone, quarantine, remote_queue,
                           keep, tcache_small, tcache_medium};
}

static AllocationPlan balanced_allocate(size_t, size_t alignment) noexcept {
    return make_plan(alignment > 16 ? StoragePreference::DirectMap : StoragePreference::Auto,
                     false, true, false, false, false, false, false, false,
                     16, configured_empty_keep(), 0, 0, 64 * 1024);
}

static ReleaseDecision balanced_free(AdaptiveHeader*) noexcept {
    return make_release(ReleaseAction::ReturnToCentralPool, false, false, false,
                        false, configured_empty_keep());
}

static AllocationPlan throughput_allocate(size_t size, size_t alignment) noexcept {
    StoragePreference storage = StoragePreference::Auto;
    if (alignment > 16) storage = StoragePreference::DirectMap;
    else if (size <= 1024) storage = StoragePreference::SizeClass;
    else if (size <= 64 * 1024) storage = StoragePreference::Span;
    return make_plan(storage, true, true, false, false, false, false, false, false,
                     96, 32, 192, 48, 256 * 1024);
}

static ReleaseDecision throughput_free(AdaptiveHeader*) noexcept {
    return make_release(ReleaseAction::Cache, false, false, false,
                        false, 32, 192, 48);
}

static AllocationPlan latency_allocate(size_t, size_t alignment) noexcept {
    return make_plan(alignment > 16 ? StoragePreference::DirectMap : StoragePreference::Auto,
                     true, true, false, false, false, false, false, false,
                     8, 6, 16, 6, 256 * 1024);
}

static ReleaseDecision latency_free(AdaptiveHeader*) noexcept {
    return make_release(ReleaseAction::Cache, false, false, false,
                        false, 6, 16, 6);
}

static AllocationPlan compact_allocate(size_t size, size_t alignment) noexcept {
    StoragePreference storage = (alignment > 16 || size > 96 * 1024)
        ? StoragePreference::DirectMap
        : StoragePreference::Auto;
    return make_plan(storage, false, false, true, false, false,
                     storage == StoragePreference::DirectMap, false, true,
                     4, 0, 0, 0, 96 * 1024);
}

static ReleaseDecision compact_free(AdaptiveHeader* hdr) noexcept {
    if (!hdr || hdr->storage == AdaptiveStorageId::DirectMap) {
        return make_release(ReleaseAction::Unmap, false, false, false,
                            false, 0);
    }
    return make_release(ReleaseAction::Purge, false, false, false,
                        false, 0);
}

static size_t align_up_policy(size_t value, size_t alignment) noexcept {
    return (value + alignment - 1) & ~(alignment - 1);
}

static bool fragmentation_prefers_direct(size_t size, size_t alignment) noexcept {
    if (alignment > 16 || size >= 96 * 1024) return true;
    if (size <= 1024 || size > 64 * 1024) return false;
    size_t rounded = align_up_policy(size, 1024);
    if (rounded <= size) return false;
    size_t waste = rounded - size;
    return size >= 16 * 1024 && waste * 100 >= size * 20;
}

static AllocationPlan fragmentation_allocate(size_t size, size_t alignment) noexcept {
    StoragePreference storage = fragmentation_prefers_direct(size, alignment)
        ? StoragePreference::DirectMap
        : StoragePreference::Auto;
    return make_plan(storage, false, true, false, false, false,
                     storage == StoragePreference::DirectMap, false, true,
                     8, 1, 0, 0, 96 * 1024);
}

static ReleaseDecision fragmentation_free(AdaptiveHeader* hdr) noexcept {
    if (hdr && hdr->storage == AdaptiveStorageId::DirectMap) {
        return make_release(ReleaseAction::Unmap, false, false, false,
                            false, 0);
    }
    return make_release(ReleaseAction::ReturnToCentralPool, false, false, false,
                        false, 1);
}

static AllocationPlan cross_thread_allocate(size_t, size_t alignment) noexcept {
    return make_plan(alignment > 16 ? StoragePreference::DirectMap : StoragePreference::Auto,
                     true, true, false, false, false, false, true, false,
                     64, 8, 96, 24, 256 * 1024);
}

static ReleaseDecision cross_thread_free(AdaptiveHeader*) noexcept {
    return make_release(ReleaseAction::Cache, false, false, false,
                        true, 8, 96, 24);
}

static AllocationPlan large_stream_allocate(size_t size, size_t alignment) noexcept {
    StoragePreference storage = StoragePreference::Auto;
    if (alignment > 16 || size >= 128 * 1024) storage = StoragePreference::Extent;
    else if (size <= 1024) storage = StoragePreference::SizeClass;
    return make_plan(storage, false, false, true, false, false,
                     storage == StoragePreference::DirectMap || storage == StoragePreference::Extent,
                     false, false, 4, 1, 0, 0, 128 * 1024);
}

static ReleaseDecision large_stream_free(AdaptiveHeader* hdr) noexcept {
    if (hdr && hdr->storage == AdaptiveStorageId::DirectMap) {
        return make_release(ReleaseAction::Cache, false, false, false,
                            false, 1);
    }
    return make_release(ReleaseAction::ReturnToCentralPool, false, false, false,
                        false, 1);
}

static AllocationPlan hardened_allocate(size_t, size_t) noexcept {
    return make_plan(StoragePreference::DirectMap, false, false, false,
                     true, true, true, false, false, 1, 0, 0, 0, 0);
}

static ReleaseDecision hardened_free(AdaptiveHeader*) noexcept {
    return make_release(ReleaseAction::Quarantine, true, true, true,
                        false, 0);
}

static const AdaptiveModePolicy g_modes[ADAPTIVE_MODE_COUNT] = {
    {AdaptiveModeId::Balanced, "balanced",
     "Default stable policy for unknown or mixed workloads.",
     balanced_allocate, balanced_free, noop, noop, noop},
    {AdaptiveModeId::ThroughputCache, "throughput_cache",
     "Throughput-first policy: cache/reuse bias, large batches, higher RSS tolerance.",
     throughput_allocate, throughput_free, noop, noop, noop},
    {AdaptiveModeId::DeterministicLatency, "deterministic_latency",
     "Tail-latency policy: stable reuse and no aggressive hot-path release.",
     latency_allocate, latency_free, noop, noop, noop},
    {AdaptiveModeId::CompactRSS, "compact_rss",
     "RSS-first policy: small retained cache and purge/unmap release decisions.",
     compact_allocate, compact_free, noop, noop, noop},
    {AdaptiveModeId::FragmentationStable, "fragmentation_stable",
     "Long-running mixed-size policy with fragmentation telemetry hooks.",
     fragmentation_allocate, fragmentation_free, noop, noop, noop},
    {AdaptiveModeId::CrossThreadMessage, "cross_thread",
     "Producer-consumer policy with owner/remote-free semantics.",
     cross_thread_allocate, cross_thread_free, noop, noop, noop},
    {AdaptiveModeId::LargeObjectStreaming, "large_object",
     "Large-object streaming policy that isolates large allocations in direct mappings.",
     large_stream_allocate, large_stream_free, noop, noop, noop},
    {AdaptiveModeId::HardenedDebug, "hardened_debug",
     "Debug/safety policy with direct-map allocation, poison, and quarantine hooks.",
     hardened_allocate, hardened_free, noop, noop, noop},
};

} // namespace

const AdaptiveModePolicy& adaptive_mode_policy(AdaptiveModeId id) noexcept {
    return g_modes[adaptive_mode_index(id)];
}

const char* adaptive_mode_name(AdaptiveModeId id) noexcept {
    return adaptive_mode_policy(id).name;
}

const char* adaptive_mode_objective(AdaptiveModeId id) noexcept {
    return adaptive_mode_policy(id).objective;
}

} // namespace my_ptmalloc
