// Adaptive telemetry and workload feature extraction.

#include "my_ptmalloc/adaptive_telemetry.h"
#include "my_ptmalloc/adaptive_runtime.h"

#include <cmath>

namespace my_ptmalloc {

namespace {

AdaptiveStats g_stats;
std::atomic<uint64_t> g_size_histogram[8]{};

static size_t storage_index(AdaptiveStorageId id) noexcept {
    switch (id) {
        case AdaptiveStorageId::SizeClass: return 0;
        case AdaptiveStorageId::Span:      return 1;
        case AdaptiveStorageId::DirectMap: return 2;
    }
    return 0;
}

static size_t size_histogram_index(size_t size) noexcept {
    if (size <= 32) return 0;
    if (size <= 64) return 1;
    if (size <= 128) return 2;
    if (size <= 256) return 3;
    if (size <= 1024) return 4;
    if (size <= 4096) return 5;
    if (size <= 64 * 1024) return 6;
    return 7;
}

static double ratio_u64(uint64_t num, uint64_t den) noexcept {
    return den == 0 ? 0.0 : static_cast<double>(num) / static_cast<double>(den);
}

} // namespace

void adaptive_telemetry_on_malloc_call() noexcept {
    g_stats.malloc_calls.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_alloc(const AllocationResult& result) noexcept {
    size_t idx = storage_index(result.storage);
    g_stats.storage[idx].alloc_count.fetch_add(1, std::memory_order_relaxed);
    g_stats.storage[idx].requested_bytes.fetch_add(result.requested, std::memory_order_relaxed);
    g_stats.storage[idx].usable_bytes.fetch_add(result.usable, std::memory_order_relaxed);
    if (result.from_cache) {
        g_stats.storage[idx].pool_hits.fetch_add(1, std::memory_order_relaxed);
    } else {
        g_stats.storage[idx].pool_misses.fetch_add(1, std::memory_order_relaxed);
    }
    if (result.used_mmap) g_stats.mmap_count.fetch_add(1, std::memory_order_relaxed);
    if (result.slow_path) g_stats.slow_path_count.fetch_add(1, std::memory_order_relaxed);
    g_size_histogram[size_histogram_index(result.requested)].fetch_add(1, std::memory_order_relaxed);
    g_stats.live_bytes.fetch_add(static_cast<int64_t>(result.usable), std::memory_order_relaxed);
    auto& mode = g_stats.mode[adaptive_mode_index(result.header->mode_id)];
    mode.alloc_count.fetch_add(1, std::memory_order_relaxed);
    mode.live_bytes.fetch_add(static_cast<int64_t>(result.usable), std::memory_order_relaxed);
}

void adaptive_telemetry_on_alloc_failure() noexcept {
    g_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_free_call() noexcept {
    g_stats.free_calls.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_free_begin(AdaptiveHeader* hdr) noexcept {
    if (!hdr) return;
    if (hdr->owner_thread == adaptive_thread_token()) {
        g_stats.same_thread_free_count.fetch_add(1, std::memory_order_relaxed);
    } else {
        g_stats.remote_free_count.fetch_add(1, std::memory_order_relaxed);
    }
    size_t idx = storage_index(hdr->storage);
    g_stats.storage[idx].free_count.fetch_add(1, std::memory_order_relaxed);
    g_stats.live_bytes.fetch_sub(static_cast<int64_t>(hdr->usable), std::memory_order_relaxed);
    auto& mode = g_stats.mode[adaptive_mode_index(hdr->mode_id)];
    mode.free_count.fetch_add(1, std::memory_order_relaxed);
    mode.live_bytes.fetch_sub(static_cast<int64_t>(hdr->usable), std::memory_order_relaxed);
}

void adaptive_telemetry_on_free_end(AdaptiveHeader*) noexcept {}

void adaptive_telemetry_on_invalid_free(void*) noexcept {
    g_stats.invalid_free_count.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_realloc() noexcept {
    g_stats.realloc_calls.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_mapped(AdaptiveModeId mode, size_t bytes) noexcept {
    g_stats.mapped_bytes.fetch_add(static_cast<int64_t>(bytes), std::memory_order_relaxed);
    g_stats.mode[adaptive_mode_index(mode)].mapped_bytes.fetch_add(static_cast<int64_t>(bytes),
                                                                   std::memory_order_relaxed);
}

void adaptive_telemetry_on_unmapped(AdaptiveModeId mode, size_t bytes) noexcept {
    g_stats.mapped_bytes.fetch_sub(static_cast<int64_t>(bytes), std::memory_order_relaxed);
    g_stats.mode[adaptive_mode_index(mode)].mapped_bytes.fetch_sub(static_cast<int64_t>(bytes),
                                                                   std::memory_order_relaxed);
}

void adaptive_telemetry_on_pool_hit(AdaptiveStorageId storage) noexcept {
    g_stats.storage[storage_index(storage)].pool_hits.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_pool_miss(AdaptiveStorageId storage) noexcept {
    g_stats.storage[storage_index(storage)].pool_misses.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_empty_counts(AdaptiveStorageId storage, uint32_t count) noexcept {
    if (storage == AdaptiveStorageId::SizeClass) {
        g_stats.empty_pages.store(count, std::memory_order_relaxed);
    } else if (storage == AdaptiveStorageId::Span) {
        g_stats.empty_spans.store(count, std::memory_order_relaxed);
    }
}

void adaptive_telemetry_on_release(AdaptiveStorageId storage, size_t bytes) noexcept {
    g_stats.release_unmapped_bytes.fetch_add(bytes, std::memory_order_relaxed);
    if (storage == AdaptiveStorageId::SizeClass) {
        g_stats.released_pages.fetch_add(1, std::memory_order_relaxed);
    } else if (storage == AdaptiveStorageId::Span) {
        g_stats.released_spans.fetch_add(1, std::memory_order_relaxed);
    }
}

void adaptive_telemetry_on_mmap() noexcept {
    g_stats.mmap_count.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_munmap() noexcept {
    g_stats.munmap_count.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_header_corruption() noexcept {
    g_stats.header_corruption_count.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_double_free() noexcept {
    g_stats.double_free_count.fetch_add(1, std::memory_order_relaxed);
}

void adaptive_telemetry_on_mode_switch() noexcept {
    g_stats.mode_switches.fetch_add(1, std::memory_order_relaxed);
}

WorkloadFeatures adaptive_extract_window_features() noexcept {
    // The current selector uses cumulative counters as a coarse window
    // approximation. The interface is deliberately window-shaped so it can
    // become a sliding delta without changing mode or selector APIs.
    WorkloadFeatures f{};
    f.alloc_calls = g_stats.malloc_calls.load(std::memory_order_relaxed);
    f.free_calls = g_stats.free_calls.load(std::memory_order_relaxed);
    f.remote_free_count = g_stats.remote_free_count.load(std::memory_order_relaxed);
    f.same_thread_free_count = g_stats.same_thread_free_count.load(std::memory_order_relaxed);
    f.mapped_bytes = static_cast<uint64_t>(g_stats.mapped_bytes.load(std::memory_order_relaxed) < 0 ? 0 :
        g_stats.mapped_bytes.load(std::memory_order_relaxed));
    f.live_bytes = static_cast<uint64_t>(g_stats.live_bytes.load(std::memory_order_relaxed) < 0 ? 0 :
        g_stats.live_bytes.load(std::memory_order_relaxed));

    uint64_t requested = 0;
    uint64_t usable = 0;
    uint64_t hits = 0;
    uint64_t misses = 0;
    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        requested += g_stats.storage[i].requested_bytes.load(std::memory_order_relaxed);
        usable += g_stats.storage[i].usable_bytes.load(std::memory_order_relaxed);
        hits += g_stats.storage[i].pool_hits.load(std::memory_order_relaxed);
        misses += g_stats.storage[i].pool_misses.load(std::memory_order_relaxed);
    }
    uint64_t small_count = g_stats.storage[storage_index(AdaptiveStorageId::SizeClass)]
        .alloc_count.load(std::memory_order_relaxed);
    uint64_t medium_count = g_stats.storage[storage_index(AdaptiveStorageId::Span)]
        .alloc_count.load(std::memory_order_relaxed);
    uint64_t large_count = g_stats.storage[storage_index(AdaptiveStorageId::DirectMap)]
        .alloc_count.load(std::memory_order_relaxed);
    uint64_t count_total = small_count + medium_count + large_count;
    uint64_t large_bytes = g_stats.storage[storage_index(AdaptiveStorageId::DirectMap)]
        .requested_bytes.load(std::memory_order_relaxed);
    uint64_t thread_frees = f.remote_free_count + f.same_thread_free_count;
    uint64_t safety = g_stats.invalid_free_count.load(std::memory_order_relaxed) +
        g_stats.double_free_count.load(std::memory_order_relaxed) +
        g_stats.header_corruption_count.load(std::memory_order_relaxed);
    uint64_t slow = g_stats.slow_path_count.load(std::memory_order_relaxed) +
        g_stats.mmap_count.load(std::memory_order_relaxed) + misses;

    f.requested_bytes = requested;
    f.usable_bytes = usable;
    f.small_object_ratio = ratio_u64(small_count, count_total);
    f.medium_object_ratio = ratio_u64(medium_count, count_total);
    f.large_object_ratio = ratio_u64(large_count, count_total);
    f.large_bytes_ratio = ratio_u64(large_bytes, requested);
    f.cache_hit_rate = ratio_u64(hits, hits + misses);
    f.reuse_rate = f.cache_hit_rate;
    f.remote_free_ratio = ratio_u64(f.remote_free_count, thread_frees);
    f.mapped_live_ratio = ratio_u64(f.mapped_bytes, f.live_bytes);
    f.retained_ratio = f.mapped_live_ratio;
    f.internal_frag_ratio = usable > requested ? ratio_u64(usable - requested, requested) : 0.0;
    f.external_frag_score = f.mapped_live_ratio > 1.0 ? f.mapped_live_ratio - 1.0 : 0.0;
    f.slow_path_ratio = ratio_u64(slow, f.alloc_calls);
    f.safety_error_rate = ratio_u64(safety, f.free_calls + f.alloc_calls);

    uint64_t hist_total = 0;
    uint64_t hist[8]{};
    for (size_t i = 0; i < 8; ++i) {
        hist[i] = g_size_histogram[i].load(std::memory_order_relaxed);
        hist_total += hist[i];
    }
    if (hist_total > 0) {
        for (size_t i = 0; i < 8; ++i) {
            if (hist[i] == 0) continue;
            double p = static_cast<double>(hist[i]) / static_cast<double>(hist_total);
            f.size_entropy -= p * (std::log(p) / std::log(2.0));
        }
    }
    return f;
}

AdaptiveStatsSnapshot adaptive_stats_snapshot() noexcept {
    AdaptiveStatsSnapshot s{};
    s.malloc_calls = g_stats.malloc_calls.load(std::memory_order_relaxed);
    s.free_calls = g_stats.free_calls.load(std::memory_order_relaxed);
    s.realloc_calls = g_stats.realloc_calls.load(std::memory_order_relaxed);
    s.failure_count = g_stats.failure_count.load(std::memory_order_relaxed);
    s.empty_pages = g_stats.empty_pages.load(std::memory_order_relaxed);
    s.empty_spans = g_stats.empty_spans.load(std::memory_order_relaxed);
    s.released_pages = g_stats.released_pages.load(std::memory_order_relaxed);
    s.released_spans = g_stats.released_spans.load(std::memory_order_relaxed);
    s.release_unmapped_bytes = g_stats.release_unmapped_bytes.load(std::memory_order_relaxed);
    s.current_mode = adaptive_current_mode();
    s.active_mode = s.current_mode;
    s.previous_mode = adaptive_previous_mode();
    s.mode_switches = g_stats.mode_switches.load(std::memory_order_relaxed);
    s.retired_mode_count = adaptive_retired_mode_count();
    for (size_t i = 0; i < ADAPTIVE_MODE_COUNT; ++i) {
        s.mode_alloc_count[i] = g_stats.mode[i].alloc_count.load(std::memory_order_relaxed);
        s.mode_free_count[i] = g_stats.mode[i].free_count.load(std::memory_order_relaxed);
        s.mode_live_bytes[i] = g_stats.mode[i].live_bytes.load(std::memory_order_relaxed);
        s.mode_mapped_bytes[i] = g_stats.mode[i].mapped_bytes.load(std::memory_order_relaxed);
    }
    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        s.storage[i].alloc_count = g_stats.storage[i].alloc_count.load(std::memory_order_relaxed);
        s.storage[i].free_count = g_stats.storage[i].free_count.load(std::memory_order_relaxed);
        s.storage[i].requested_bytes = g_stats.storage[i].requested_bytes.load(std::memory_order_relaxed);
        s.storage[i].usable_bytes = g_stats.storage[i].usable_bytes.load(std::memory_order_relaxed);
        s.storage[i].pool_hits = g_stats.storage[i].pool_hits.load(std::memory_order_relaxed);
        s.storage[i].pool_misses = g_stats.storage[i].pool_misses.load(std::memory_order_relaxed);
    }
    s.live_bytes = g_stats.live_bytes.load(std::memory_order_relaxed);
    s.mapped_bytes = g_stats.mapped_bytes.load(std::memory_order_relaxed);
    WorkloadFeatures f = adaptive_extract_window_features();
    s.remote_free_ratio = f.remote_free_ratio;
    s.size_entropy = f.size_entropy;
    s.large_bytes_ratio = f.large_bytes_ratio;
    s.mapped_live_ratio = f.mapped_live_ratio;
    s.fragmentation_estimate = f.internal_frag_ratio;
    s.slow_path_ratio = f.slow_path_ratio;
    s.double_free_count = g_stats.double_free_count.load(std::memory_order_relaxed);
    s.invalid_free_count = g_stats.invalid_free_count.load(std::memory_order_relaxed);
    s.header_corruption_count = g_stats.header_corruption_count.load(std::memory_order_relaxed);
    return s;
}

void adaptive_stats_reset() noexcept {
    g_stats.malloc_calls.store(0, std::memory_order_relaxed);
    g_stats.free_calls.store(0, std::memory_order_relaxed);
    g_stats.realloc_calls.store(0, std::memory_order_relaxed);
    g_stats.failure_count.store(0, std::memory_order_relaxed);
    g_stats.empty_pages.store(0, std::memory_order_relaxed);
    g_stats.empty_spans.store(0, std::memory_order_relaxed);
    g_stats.released_pages.store(0, std::memory_order_relaxed);
    g_stats.released_spans.store(0, std::memory_order_relaxed);
    g_stats.release_unmapped_bytes.store(0, std::memory_order_relaxed);
    g_stats.mode_switches.store(0, std::memory_order_relaxed);
    g_stats.remote_free_count.store(0, std::memory_order_relaxed);
    g_stats.same_thread_free_count.store(0, std::memory_order_relaxed);
    g_stats.invalid_free_count.store(0, std::memory_order_relaxed);
    g_stats.double_free_count.store(0, std::memory_order_relaxed);
    g_stats.header_corruption_count.store(0, std::memory_order_relaxed);
    g_stats.slow_path_count.store(0, std::memory_order_relaxed);
    g_stats.mmap_count.store(0, std::memory_order_relaxed);
    g_stats.munmap_count.store(0, std::memory_order_relaxed);
    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        g_stats.storage[i].alloc_count.store(0, std::memory_order_relaxed);
        g_stats.storage[i].free_count.store(0, std::memory_order_relaxed);
        g_stats.storage[i].requested_bytes.store(0, std::memory_order_relaxed);
        g_stats.storage[i].usable_bytes.store(0, std::memory_order_relaxed);
        g_stats.storage[i].pool_hits.store(0, std::memory_order_relaxed);
        g_stats.storage[i].pool_misses.store(0, std::memory_order_relaxed);
    }
    for (size_t i = 0; i < ADAPTIVE_MODE_COUNT; ++i) {
        g_stats.mode[i].alloc_count.store(0, std::memory_order_relaxed);
        g_stats.mode[i].free_count.store(0, std::memory_order_relaxed);
        g_stats.mode[i].live_bytes.store(0, std::memory_order_relaxed);
        g_stats.mode[i].mapped_bytes.store(0, std::memory_order_relaxed);
    }
    g_stats.live_bytes.store(0, std::memory_order_relaxed);
    g_stats.mapped_bytes.store(0, std::memory_order_relaxed);
    for (size_t i = 0; i < 8; ++i) {
        g_size_histogram[i].store(0, std::memory_order_relaxed);
    }
}

} // namespace my_ptmalloc
