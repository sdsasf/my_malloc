#pragma once
// Independent adaptive allocator backend.
// This is NOT a dispatcher over teaching allocators - it is a standalone
// allocator with its own metadata, ownership, storage helpers, and
// runtime control state.

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {

struct AdaptivePage;

// --- Adaptive mode IDs ------------------------------------------------------
//
// AdaptiveModeId is the top-level adaptive allocator abstraction. malloc uses
// the current active mode; free/realloc/usable_size route by the allocation-time
// mode_id stored in AdaptiveHeader. The PooledSmall/PooledMedium/DirectMap
// mechanisms below are implementation details used inside modes.

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

// --- Internal storage helper IDs -------------------------------------------
//
// These values are implementation details used inside AdaptiveMode
// implementations:
// - PooledSmall: page + size-class mechanism
// - PooledMedium: span + class mechanism
// - DirectMap: direct mmap mechanism

enum class AdaptiveStorageId : uint8_t {
    PooledSmall  = 1,   // size <= 1024
    PooledMedium = 2,   // 1025 .. 64 KiB
    DirectMap  = 3,   // > 64 KiB  (direct mmap)
};

// --- Adaptive block header (prepended to every allocation) -----------------

static constexpr uint32_t ADAPTIVE_MAGIC = 0xADA9'B10C;  // "ADA-BLOC"

struct AdaptiveHeader {
    uint32_t           magic;         // ADAPTIVE_MAGIC
    AdaptiveStorageId storage;       // allocation-time storage helper id
    AdaptiveModeId     mode_id;       // allocation-time adaptive mode id
    uint8_t            flags;         // bit 0: memalign allocation
    uint8_t            _pad;          // alignment padding
    uint32_t           config_version;// runtime config version used at allocation
    size_t             requested;     // user-requested size
    size_t             usable;        // usable bytes after header
    size_t             mapped_size;   // total mmap region size
    void*              region_base;   // actual mmap base (may differ from this for memalign)
    AdaptiveHeader*    registry_prev; // intrusive ownership registry link
    AdaptiveHeader*    registry_next; // intrusive ownership registry link
    AdaptivePage*      owner_page;    // adaptive page/span for pooled blocks
    uint64_t           owner_thread;  // allocation-time thread token
};

// Header is placed before user data. User pointer = (char*)hdr + ADAPTIVE_HDR_OFFSET.
static_assert(sizeof(AdaptiveHeader) <= 96, "AdaptiveHeader must fit in the fixed offset");
static constexpr size_t ADAPTIVE_HDR_OFFSET = 96;

// --- Internal helper-path stats --------------------------------------------

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

    // Internal helper paths, not adaptive architectures.
    static constexpr size_t NUM_STORAGE_HELPERS = 3;
    AdaptiveStorageStats storage[NUM_STORAGE_HELPERS]; // pooled-small, pooled-medium, direct-map

    // aggregate
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
