// Adaptive telemetry and workload feature extraction.

#include "my_ptmalloc/adaptive_telemetry.h"
#include "my_ptmalloc/adaptive_runtime.h"

#include <cmath>
#include <mutex>

namespace my_ptmalloc {

namespace {

static constexpr size_t SIZE_BUCKET_COUNT = 10;
static constexpr size_t FIRST_LARGE_SIZE_BUCKET = 8; // request size > 256 KiB

AdaptiveStats g_stats;
std::atomic<uint64_t> g_size_histogram[SIZE_BUCKET_COUNT]{};
std::atomic<uint64_t> g_size_bytes_histogram[SIZE_BUCKET_COUNT]{};

struct TelemetryCounters {
    uint64_t malloc_calls = 0;
    uint64_t free_calls = 0;
    uint64_t invalid_free_count = 0;
    uint64_t double_free_count = 0;
    uint64_t header_corruption_count = 0;
    uint64_t slow_path_count = 0;
    uint64_t mmap_count = 0;
    uint64_t remote_free_count = 0;
    uint64_t same_thread_free_count = 0;
    uint64_t storage_alloc_count[AdaptiveStats::NUM_STORAGE_HELPERS]{};
    uint64_t storage_requested_bytes[AdaptiveStats::NUM_STORAGE_HELPERS]{};
    uint64_t storage_usable_bytes[AdaptiveStats::NUM_STORAGE_HELPERS]{};
    uint64_t storage_pool_hits[AdaptiveStats::NUM_STORAGE_HELPERS]{};
    uint64_t storage_pool_misses[AdaptiveStats::NUM_STORAGE_HELPERS]{};
    uint64_t size_histogram[SIZE_BUCKET_COUNT]{};
    uint64_t size_bytes_histogram[SIZE_BUCKET_COUNT]{};
    uint64_t mapped_bytes = 0;
    uint64_t live_bytes = 0;
};

std::mutex g_window_mutex;
TelemetryCounters g_window_base;

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
    if (size <= 256 * 1024) return 7;
    if (size <= 1024 * 1024) return 8;
    return 9;
}

static double ratio_u64(uint64_t num, uint64_t den) noexcept {
    return den == 0 ? 0.0 : static_cast<double>(num) / static_cast<double>(den);
}

static uint64_t nonnegative_i64(int64_t value) noexcept {
    return value < 0 ? 0 : static_cast<uint64_t>(value);
}

static uint64_t delta_u64(uint64_t current, uint64_t base) noexcept {
    return current >= base ? current - base : current;
}

static TelemetryCounters read_counters() noexcept {
    TelemetryCounters c{};
    c.malloc_calls = g_stats.malloc_calls.load(std::memory_order_relaxed);
    c.free_calls = g_stats.free_calls.load(std::memory_order_relaxed);
    c.invalid_free_count = g_stats.invalid_free_count.load(std::memory_order_relaxed);
    c.double_free_count = g_stats.double_free_count.load(std::memory_order_relaxed);
    c.header_corruption_count = g_stats.header_corruption_count.load(std::memory_order_relaxed);
    c.slow_path_count = g_stats.slow_path_count.load(std::memory_order_relaxed);
    c.mmap_count = g_stats.mmap_count.load(std::memory_order_relaxed);
    c.remote_free_count = g_stats.remote_free_count.load(std::memory_order_relaxed);
    c.same_thread_free_count = g_stats.same_thread_free_count.load(std::memory_order_relaxed);
    c.mapped_bytes = nonnegative_i64(g_stats.mapped_bytes.load(std::memory_order_relaxed));
    c.live_bytes = nonnegative_i64(g_stats.live_bytes.load(std::memory_order_relaxed));
    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        c.storage_alloc_count[i] = g_stats.storage[i].alloc_count.load(std::memory_order_relaxed);
        c.storage_requested_bytes[i] = g_stats.storage[i].requested_bytes.load(std::memory_order_relaxed);
        c.storage_usable_bytes[i] = g_stats.storage[i].usable_bytes.load(std::memory_order_relaxed);
        c.storage_pool_hits[i] = g_stats.storage[i].pool_hits.load(std::memory_order_relaxed);
        c.storage_pool_misses[i] = g_stats.storage[i].pool_misses.load(std::memory_order_relaxed);
    }
    for (size_t i = 0; i < SIZE_BUCKET_COUNT; ++i) {
        c.size_histogram[i] = g_size_histogram[i].load(std::memory_order_relaxed);
        c.size_bytes_histogram[i] = g_size_bytes_histogram[i].load(std::memory_order_relaxed);
    }
    return c;
}

static TelemetryCounters delta_counters(const TelemetryCounters& current,
                                        const TelemetryCounters& base) noexcept {
    TelemetryCounters d{};
    d.malloc_calls = delta_u64(current.malloc_calls, base.malloc_calls);
    d.free_calls = delta_u64(current.free_calls, base.free_calls);
    d.invalid_free_count = delta_u64(current.invalid_free_count, base.invalid_free_count);
    d.double_free_count = delta_u64(current.double_free_count, base.double_free_count);
    d.header_corruption_count = delta_u64(current.header_corruption_count, base.header_corruption_count);
    d.slow_path_count = delta_u64(current.slow_path_count, base.slow_path_count);
    d.mmap_count = delta_u64(current.mmap_count, base.mmap_count);
    d.remote_free_count = delta_u64(current.remote_free_count, base.remote_free_count);
    d.same_thread_free_count = delta_u64(current.same_thread_free_count, base.same_thread_free_count);
    d.mapped_bytes = current.mapped_bytes;
    d.live_bytes = current.live_bytes;
    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        d.storage_alloc_count[i] = delta_u64(current.storage_alloc_count[i], base.storage_alloc_count[i]);
        d.storage_requested_bytes[i] =
            delta_u64(current.storage_requested_bytes[i], base.storage_requested_bytes[i]);
        d.storage_usable_bytes[i] = delta_u64(current.storage_usable_bytes[i], base.storage_usable_bytes[i]);
        d.storage_pool_hits[i] = delta_u64(current.storage_pool_hits[i], base.storage_pool_hits[i]);
        d.storage_pool_misses[i] = delta_u64(current.storage_pool_misses[i], base.storage_pool_misses[i]);
    }
    for (size_t i = 0; i < SIZE_BUCKET_COUNT; ++i) {
        d.size_histogram[i] = delta_u64(current.size_histogram[i], base.size_histogram[i]);
        d.size_bytes_histogram[i] =
            delta_u64(current.size_bytes_histogram[i], base.size_bytes_histogram[i]);
    }
    return d;
}

static WorkloadFeatures features_from_counters(const TelemetryCounters& c) noexcept {
    WorkloadFeatures f{};
    f.alloc_calls = c.malloc_calls;
    f.free_calls = c.free_calls;
    f.remote_free_count = c.remote_free_count;
    f.same_thread_free_count = c.same_thread_free_count;
    f.mapped_bytes = c.mapped_bytes;
    f.live_bytes = c.live_bytes;

    uint64_t requested = 0;
    uint64_t usable = 0;
    uint64_t hits = 0;
    uint64_t misses = 0;
    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        requested += c.storage_requested_bytes[i];
        usable += c.storage_usable_bytes[i];
        hits += c.storage_pool_hits[i];
        misses += c.storage_pool_misses[i];
    }
    // Size profile is based on requested allocation size, not the storage path
    // selected by the current mode. Otherwise a DirectMap-heavy mode can create
    // a feedback loop where its own storage decision makes the next window look
    // like a large-object workload.
    uint64_t requested_by_size = 0;
    for (size_t i = 0; i < SIZE_BUCKET_COUNT; ++i) requested_by_size += c.size_bytes_histogram[i];
    if (requested_by_size > 0) requested = requested_by_size;
    uint64_t small_count = 0;
    uint64_t medium_count = 0;
    uint64_t large_count = 0;
    uint64_t large_bytes = 0;
    for (size_t i = 0; i < SIZE_BUCKET_COUNT; ++i) {
        if (i <= 4) {
            small_count += c.size_histogram[i];
        } else if (i < FIRST_LARGE_SIZE_BUCKET) {
            medium_count += c.size_histogram[i];
        } else {
            large_count += c.size_histogram[i];
            large_bytes += c.size_bytes_histogram[i];
        }
    }
    uint64_t count_total = small_count + medium_count + large_count;
    uint64_t thread_frees = f.remote_free_count + f.same_thread_free_count;
    uint64_t safety = c.invalid_free_count + c.double_free_count + c.header_corruption_count;
    uint64_t slow = c.slow_path_count + c.mmap_count + misses;

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
    for (size_t i = 0; i < SIZE_BUCKET_COUNT; ++i) hist_total += c.size_histogram[i];
    if (hist_total > 0) {
        for (size_t i = 0; i < SIZE_BUCKET_COUNT; ++i) {
            if (c.size_histogram[i] == 0) continue;
            double p = static_cast<double>(c.size_histogram[i]) / static_cast<double>(hist_total);
            f.size_entropy -= p * (std::log(p) / std::log(2.0));
        }
    }
    return f;
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
    size_t size_bucket = size_histogram_index(result.requested);
    g_size_histogram[size_bucket].fetch_add(1, std::memory_order_relaxed);
    g_size_bytes_histogram[size_bucket].fetch_add(result.requested, std::memory_order_relaxed);
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
    TelemetryCounters current = read_counters();
    std::lock_guard<std::mutex> lock(g_window_mutex);
    TelemetryCounters delta = delta_counters(current, g_window_base);
    g_window_base = current;
    return features_from_counters(delta);
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
    WorkloadFeatures f = features_from_counters(read_counters());
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
    {
        std::lock_guard<std::mutex> lock(g_window_mutex);
        g_window_base = TelemetryCounters{};
    }
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
    for (size_t i = 0; i < SIZE_BUCKET_COUNT; ++i) {
        g_size_histogram[i].store(0, std::memory_order_relaxed);
        g_size_bytes_histogram[i].store(0, std::memory_order_relaxed);
    }
}

} // namespace my_ptmalloc
