#pragma once
// Independent adaptive allocator backend.
// This is NOT a dispatcher over teaching allocators - it is a standalone
// allocator with its own metadata, ownership, internal strategies, and policy.

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {

struct AdaptivePage;

// --- Internal strategy IDs -------------------------------------------------

enum class AdaptiveStrategyId : uint8_t {
    SmallObject  = 1,   // size <= 1024
    MediumObject = 2,   // 1025 .. 64 KiB
    LargeObject  = 3,   // > 64 KiB  (direct mmap)
};

// --- Policy kinds (adaptive-internal, NOT the old runtime AdaptivePolicyKind) -

enum class AdaptiveInternalPolicy : uint8_t {
    Heuristic        = 0,   // size-based routing (default baseline)
    FixedSmall       = 1,   // force SmallObject with safe fallback
    FixedMedium      = 2,   // force MediumObject with safe fallback
    FixedLarge       = 3,   // force LargeObject
    RoundRobin       = 4,   // cycle strategies for ownership stress test
    EpsilonGreedy    = 5,   // classic multi-armed bandit baseline
    Ucb1             = 6,   // upper confidence bound bandit
    ThompsonSampling = 7,   // lightweight Thompson-style bandit
};

// --- Adaptive block header (prepended to every allocation) -----------------

static constexpr uint32_t ADAPTIVE_MAGIC = 0xADA9'B10C;  // "ADA-BLOC"

struct AdaptiveHeader {
    uint32_t           magic;         // ADAPTIVE_MAGIC
    AdaptiveStrategyId strategy;      // which internal strategy allocated this
    uint8_t            flags;         // bit 0: memalign allocation
    uint16_t           _pad;          // alignment padding
    size_t             requested;     // user-requested size
    size_t             usable;        // usable bytes after header
    size_t             mapped_size;   // total mmap region size
    void*              region_base;   // actual mmap base (may differ from this for memalign)
    AdaptiveHeader*    registry_prev; // intrusive ownership registry link
    AdaptiveHeader*    registry_next; // intrusive ownership registry link
    AdaptivePage*      owner_page;    // adaptive page/span for pooled blocks
};

// Header is placed before user data. User pointer = (char*)hdr + ADAPTIVE_HDR_OFFSET.
static_assert(sizeof(AdaptiveHeader) <= 80, "AdaptiveHeader must fit in the fixed offset");
static constexpr size_t ADAPTIVE_HDR_OFFSET = 80;

// --- Observation / extensibility hook --------------------------------------

struct AdaptiveObservation {
    size_t             requested_size;
    AdaptiveStrategyId chosen_strategy;
    AdaptiveInternalPolicy policy;
    uint64_t           live_bytes;
    uint64_t           mapped_bytes;
    uint64_t           strategy_trials;
    uint64_t           strategy_successes;
    uint64_t           avg_alloc_latency_ns;
    uint64_t           pool_hits;
    uint64_t           pool_misses;
    bool               success;
    // Future: phase id, remote-free ratio, contention level, etc.
};

// --- Per-strategy stats ----------------------------------------------------

struct AdaptiveStrategyStats {
    std::atomic<uint64_t> alloc_count{0};
    std::atomic<uint64_t> free_count{0};
    std::atomic<uint64_t> requested_bytes{0};
    std::atomic<uint64_t> usable_bytes{0};
    std::atomic<uint64_t> policy_trials{0};
    std::atomic<uint64_t> policy_successes{0};
    std::atomic<uint64_t> policy_failures{0};
    std::atomic<uint64_t> avg_alloc_latency_ns{0};
    std::atomic<uint64_t> pool_hits{0};
    std::atomic<uint64_t> pool_misses{0};
};

struct AdaptiveStats {
    std::atomic<uint64_t> malloc_calls{0};
    std::atomic<uint64_t> free_calls{0};
    std::atomic<uint64_t> realloc_calls{0};
    std::atomic<uint64_t> failure_count{0};
    std::atomic<uint64_t> policy_decisions{0};

    // per-strategy
    static constexpr size_t NUM_STRATEGIES = 3;
    AdaptiveStrategyStats strategy[NUM_STRATEGIES]; // index 0=Small,1=Medium,2=Large

    // aggregate
    std::atomic<int64_t> live_bytes{0};
    std::atomic<int64_t> mapped_bytes{0};
};

struct AdaptiveStatsSnapshot {
    uint64_t malloc_calls;
    uint64_t free_calls;
    uint64_t realloc_calls;
    uint64_t failure_count;
    uint64_t policy_decisions;

    struct PerStrategy {
        uint64_t alloc_count;
        uint64_t free_count;
        uint64_t requested_bytes;
        uint64_t usable_bytes;
        uint64_t policy_trials;
        uint64_t policy_successes;
        uint64_t policy_failures;
        uint64_t avg_alloc_latency_ns;
        uint64_t pool_hits;
        uint64_t pool_misses;
    };
    PerStrategy strategy[3];

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

// Policy query
[[nodiscard]] AdaptiveInternalPolicy adaptive_current_policy() noexcept;
[[nodiscard]] const char* adaptive_policy_name(AdaptiveInternalPolicy p) noexcept;

} // namespace my_ptmalloc
