#pragma once
// Shared adaptive allocator types. These definitions are intentionally free of
// policy logic: they are used by the shared memory layer, mode layer,
// telemetry, and selector.

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {

struct AdaptivePage;

enum class AdaptiveModeId : uint8_t {
    Balanced = 0,
    ThroughputCache = 1,
    DeterministicLatency = 2,
    CompactRSS = 3,
    FragmentationStable = 4,
    CrossThreadMessage = 5,
    LargeObjectStreaming = 6,
    HardenedDebug = 7,
};

static constexpr size_t ADAPTIVE_MODE_COUNT = 8;

enum class AdaptiveStorageId : uint8_t {
    SizeClass = 1,  // size-class pages for small objects
    Span      = 2,  // span-backed storage for medium objects
    DirectMap = 3,  // extent/direct mapping for large/aligned/debug objects
};

static constexpr uint32_t ADAPTIVE_MAGIC = 0xADA9'B10C;

struct AdaptiveHeader {
    uint32_t           magic;
    AdaptiveStorageId storage;
    AdaptiveModeId    mode_id;
    uint8_t           flags;
    uint8_t           _pad;
    uint32_t          config_version;
    size_t            requested;
    size_t            usable;
    size_t            mapped_size;
    void*             region_base;
    AdaptiveHeader*   registry_prev;
    AdaptiveHeader*   registry_next;
    AdaptivePage*     owner_page;
    uint64_t          owner_thread;
    uint64_t          alloc_epoch;
};

static_assert(sizeof(AdaptiveHeader) <= 112, "AdaptiveHeader must fit in the fixed offset");
static constexpr size_t ADAPTIVE_HDR_OFFSET = 112;

struct AdaptiveStorageStats {
    std::atomic<uint64_t> alloc_count{0};
    std::atomic<uint64_t> free_count{0};
    std::atomic<uint64_t> requested_bytes{0};
    std::atomic<uint64_t> usable_bytes{0};
    std::atomic<uint64_t> pool_hits{0};
    std::atomic<uint64_t> pool_misses{0};
};

struct AdaptiveStats {
    std::atomic<uint64_t> malloc_calls{0};
    std::atomic<uint64_t> free_calls{0};
    std::atomic<uint64_t> realloc_calls{0};
    std::atomic<uint64_t> failure_count{0};
    std::atomic<uint64_t> empty_pages{0};
    std::atomic<uint64_t> empty_spans{0};
    std::atomic<uint64_t> released_pages{0};
    std::atomic<uint64_t> released_spans{0};
    std::atomic<uint64_t> release_unmapped_bytes{0};
    std::atomic<uint64_t> mode_switches{0};
    std::atomic<uint64_t> remote_free_count{0};
    std::atomic<uint64_t> same_thread_free_count{0};
    std::atomic<uint64_t> invalid_free_count{0};
    std::atomic<uint64_t> double_free_count{0};
    std::atomic<uint64_t> header_corruption_count{0};
    std::atomic<uint64_t> slow_path_count{0};
    std::atomic<uint64_t> mmap_count{0};
    std::atomic<uint64_t> munmap_count{0};

    static constexpr size_t NUM_STORAGE_HELPERS = 3;
    AdaptiveStorageStats storage[NUM_STORAGE_HELPERS];

    std::atomic<int64_t> live_bytes{0};
    std::atomic<int64_t> mapped_bytes{0};

    struct ModeStats {
        std::atomic<uint64_t> alloc_count{0};
        std::atomic<uint64_t> free_count{0};
        std::atomic<int64_t> live_bytes{0};
        std::atomic<int64_t> mapped_bytes{0};
    };
    ModeStats mode[ADAPTIVE_MODE_COUNT];
};

struct WorkloadFeatures {
    double small_object_ratio;
    double medium_object_ratio;
    double large_object_ratio;
    double large_bytes_ratio;
    double size_entropy;
    double cache_hit_rate;
    double reuse_rate;
    double remote_free_ratio;
    double mapped_live_ratio;
    double retained_ratio;
    double internal_frag_ratio;
    double external_frag_score;
    double slow_path_ratio;
    double safety_error_rate;
    uint64_t alloc_calls;
    uint64_t free_calls;
    uint64_t requested_bytes;
    uint64_t usable_bytes;
    uint64_t mapped_bytes;
    uint64_t live_bytes;
    uint64_t remote_free_count;
    uint64_t same_thread_free_count;
};

struct AdaptiveStatsSnapshot {
    uint64_t malloc_calls;
    uint64_t free_calls;
    uint64_t realloc_calls;
    uint64_t failure_count;
    uint64_t empty_pages;
    uint64_t empty_spans;
    uint64_t released_pages;
    uint64_t released_spans;
    uint64_t release_unmapped_bytes;
    AdaptiveModeId current_mode;
    AdaptiveModeId active_mode;
    AdaptiveModeId previous_mode;
    uint64_t mode_switches;
    uint64_t retired_mode_count;
    uint64_t mode_alloc_count[ADAPTIVE_MODE_COUNT];
    uint64_t mode_free_count[ADAPTIVE_MODE_COUNT];
    int64_t mode_live_bytes[ADAPTIVE_MODE_COUNT];
    int64_t mode_mapped_bytes[ADAPTIVE_MODE_COUNT];
    double remote_free_ratio;
    double size_entropy;
    double large_bytes_ratio;
    double mapped_live_ratio;
    double fragmentation_estimate;
    double slow_path_ratio;
    uint64_t double_free_count;
    uint64_t invalid_free_count;
    uint64_t header_corruption_count;

    struct PerStorage {
        uint64_t alloc_count;
        uint64_t free_count;
        uint64_t requested_bytes;
        uint64_t usable_bytes;
        uint64_t pool_hits;
        uint64_t pool_misses;
    };
    PerStorage storage[3];

    int64_t live_bytes;
    int64_t mapped_bytes;
};

} // namespace my_ptmalloc
