// Independent adaptive allocator backend.
// All allocations carry an AdaptiveHeader for ownership, allocation-time mode,
// and storage-helper tracking. AdaptiveMode is the top-level abstraction; the
// storage helper ids refer only to pooled-small, pooled-medium, and direct-map
// implementation paths inside a mode.
// V2: small/medium use adaptive-owned pages/spans and size-class free lists;
// large/aligned allocations use direct mmap.

#include "my_ptmalloc/adaptive_allocator.h"

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <pthread.h>
#include <sys/mman.h>

namespace my_ptmalloc {

// --- Global state ----------------------------------------------------------

static AdaptiveStats g_adaptive_stats;
static pthread_once_t g_adaptive_config_once = PTHREAD_ONCE_INIT;
static std::mutex g_adaptive_registry_mutex;
static AdaptiveHeader* g_adaptive_registry_head = nullptr;

struct AdaptivePage {
    AdaptiveStorageId storage;
    AdaptiveModeId mode_id;
    uint32_t config_version;
    uint16_t class_index;
    uint16_t _pad;
    size_t block_usable;
    size_t block_stride;
    size_t mapped_size;
    uint32_t capacity;
    uint32_t live_count;
    AdaptiveHeader* free_list;
    AdaptivePage* next;
};

namespace {

constexpr uint8_t ADAPTIVE_FLAG_MEMALIGN = 1u << 0;
constexpr uint8_t ADAPTIVE_FLAG_POOLED   = 1u << 1;
constexpr size_t ADAPTIVE_SMALL_MAX = 1024;
constexpr size_t ADAPTIVE_MEDIUM_MAX = 64 * 1024;
constexpr size_t ADAPTIVE_SMALL_CLASS_STEP = 16;
constexpr size_t ADAPTIVE_MEDIUM_CLASS_STEP = 1024;
constexpr size_t ADAPTIVE_SMALL_CLASS_COUNT = ADAPTIVE_SMALL_MAX / ADAPTIVE_SMALL_CLASS_STEP;
constexpr size_t ADAPTIVE_MEDIUM_CLASS_COUNT =
    (ADAPTIVE_MEDIUM_MAX - ADAPTIVE_SMALL_MAX) / ADAPTIVE_MEDIUM_CLASS_STEP;
constexpr size_t ADAPTIVE_SMALL_PAGE_SIZE = 64 * 1024;
constexpr size_t ADAPTIVE_MEDIUM_SPAN_SIZE = 256 * 1024;
constexpr uint32_t ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT = 2;
constexpr uint32_t ADAPTIVE_DEFAULT_MODE_WINDOW = 4096;
constexpr uint32_t ADAPTIVE_DEFAULT_MODE_COOLDOWN = 2;
constexpr size_t ADAPTIVE_PAGE_TABLE_SIZE = 262144;
constexpr size_t ADAPTIVE_PAGE_TABLE_PROBE = 16;
constexpr uintptr_t ADAPTIVE_PAGE_TOMBSTONE = static_cast<uintptr_t>(-1);

struct AdaptiveRuntimeConfig {
    uint32_t version;
    size_t small_page_size;
    size_t medium_span_size;
    uint32_t empty_cache_limit;
    AdaptiveModeId configured_mode;
    uint32_t mode_window;
    uint32_t mode_cooldown;
};

enum class AdaptiveModeSelectorKind : uint8_t {
    Rule = 0,
    Fixed = 1,
    Manual = 2,
};

enum class AdaptiveModeState : uint8_t {
    Inactive = 0,
    Active = 1,
    Retired = 2,
};

struct WorkloadSignature {
    uint64_t alloc_calls;
    uint64_t free_calls;
    uint64_t requested_bytes;
    uint64_t usable_bytes;
    uint64_t large_bytes;
    uint64_t remote_free_count;
    uint64_t same_thread_free_count;
    uint64_t pool_hits;
    uint64_t pool_misses;
    uint64_t mmap_count;
    uint64_t munmap_count;
    uint64_t slow_path_count;
    uint64_t invalid_free_count;
    uint64_t double_free_count;
    uint64_t size_hist[8];
    int64_t live_bytes;
    int64_t mapped_bytes;
    double size_entropy;
    double large_bytes_ratio;
    double mapped_live_ratio;
    double fragmentation_estimate;
    double remote_free_ratio;
    double slow_path_ratio;
    double reuse_rate;
};

struct AdaptiveModeOps {
    AdaptiveModeId id;
    const char* name;
    const char* objective;
    AdaptiveStorageId (*select_storage)(size_t) noexcept;
    void* (*allocate)(size_t, AdaptiveModeId, AdaptiveStorageId*) noexcept;
    void (*deallocate)(AdaptiveHeader*) noexcept;
    void* (*reallocate)(void*, size_t) noexcept;
    size_t (*usable_size)(AdaptiveHeader*) noexcept;
    void (*on_activate)(AdaptiveModeId) noexcept;
    void (*on_retire)(AdaptiveModeId) noexcept;
};

std::mutex g_adaptive_config_mutex;
std::mutex g_small_class_mutexes[ADAPTIVE_SMALL_CLASS_COUNT];
std::mutex g_medium_class_mutexes[ADAPTIVE_MEDIUM_CLASS_COUNT];
AdaptivePage* g_small_pages[ADAPTIVE_SMALL_CLASS_COUNT]{};
AdaptivePage* g_medium_pages[ADAPTIVE_MEDIUM_CLASS_COUNT]{};
std::atomic<uint32_t> g_config_version{1};
std::atomic<size_t> g_config_small_page_size{ADAPTIVE_SMALL_PAGE_SIZE};
std::atomic<size_t> g_config_medium_span_size{ADAPTIVE_MEDIUM_SPAN_SIZE};
std::atomic<uint32_t> g_config_empty_cache_limit{ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT};
std::atomic<uint8_t> g_config_mode{static_cast<uint8_t>(AdaptiveModeId::Balanced)};
std::atomic<uint8_t> g_mode_selector_kind{static_cast<uint8_t>(AdaptiveModeSelectorKind::Rule)};
std::atomic<uint32_t> g_config_mode_window{ADAPTIVE_DEFAULT_MODE_WINDOW};
std::atomic<uint32_t> g_config_mode_cooldown{ADAPTIVE_DEFAULT_MODE_COOLDOWN};
std::atomic<uint8_t> g_active_mode{static_cast<uint8_t>(AdaptiveModeId::Balanced)};
std::atomic<uint8_t> g_previous_mode{static_cast<uint8_t>(AdaptiveModeId::Balanced)};
std::atomic<uint64_t> g_mode_decision_counter{0};
std::atomic<uint64_t> g_mode_cooldown_until{0};
std::atomic<uint8_t> g_mode_states[ADAPTIVE_MODE_COUNT]{};
std::atomic<uint64_t> g_size_histogram[8]{};

std::atomic<uintptr_t> g_adaptive_page_table[ADAPTIVE_PAGE_TABLE_SIZE]{};

} // namespace

// --- Policy initialization ------------------------------------------------

static bool adaptive_streq(const char* a, const char* b) noexcept {
    return a && std::strcmp(a, b) == 0;
}

static bool adaptive_mode_alias(const char* env, const char* a, const char* b) noexcept {
    return adaptive_streq(env, a) || adaptive_streq(env, b);
}

static size_t mode_index(AdaptiveModeId id) noexcept {
    uint8_t value = static_cast<uint8_t>(id);
    return value < ADAPTIVE_MODE_COUNT ? value : 0;
}

static bool valid_mode_id(AdaptiveModeId id) noexcept {
    return mode_index(id) == static_cast<size_t>(static_cast<uint8_t>(id));
}

static uint64_t current_thread_token() noexcept {
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pthread_self()));
}

static size_t size_histogram_index(size_t size) noexcept {
    if (size <= 32) return 0;
    if (size <= 64) return 1;
    if (size <= 128) return 2;
    if (size <= 256) return 3;
    if (size <= 1024) return 4;
    if (size <= 4096) return 5;
    if (size <= ADAPTIVE_MEDIUM_MAX) return 6;
    return 7;
}

static AdaptiveModeId parse_mode(const char* env, bool* is_auto = nullptr) noexcept {
    if (is_auto) *is_auto = false;
    if (!env || !*env || adaptive_streq(env, "auto")) {
        if (is_auto) *is_auto = true;
        return AdaptiveModeId::Balanced;
    }
    if (adaptive_streq(env, "balanced")) return AdaptiveModeId::Balanced;
    if (adaptive_mode_alias(env, "throughput_cache", "throughput")) return AdaptiveModeId::ThroughputCache;
    if (adaptive_mode_alias(env, "deterministic_latency", "low_latency")) return AdaptiveModeId::DeterministicLatency;
    if (adaptive_mode_alias(env, "compact_rss", "low_rss")) return AdaptiveModeId::CompactRSS;
    if (adaptive_mode_alias(env, "fragmentation_stable", "fragmentation")) return AdaptiveModeId::FragmentationStable;
    if (adaptive_mode_alias(env, "cross_thread", "cross_thread_message")) return AdaptiveModeId::CrossThreadMessage;
    if (adaptive_mode_alias(env, "large_object", "large_object_streaming")) return AdaptiveModeId::LargeObjectStreaming;
    if (adaptive_mode_alias(env, "hardened_debug", "debug")) return AdaptiveModeId::HardenedDebug;
    return AdaptiveModeId::Balanced;
}

static AdaptiveModeSelectorKind parse_mode_selector(const char* env) noexcept {
    if (adaptive_streq(env, "fixed")) return AdaptiveModeSelectorKind::Fixed;
    if (adaptive_streq(env, "manual")) return AdaptiveModeSelectorKind::Manual;
    return AdaptiveModeSelectorKind::Rule;
}

static uint64_t adaptive_parse_u64_env(const char* name,
                                       uint64_t fallback,
                                       uint64_t min_value,
                                       uint64_t max_value) noexcept {
    const char* env = std::getenv(name);
    if (!env || !*env) return fallback;
    char* end = nullptr;
    uint64_t value = std::strtoull(env, &end, 10);
    if (end == env) return fallback;
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static size_t adaptive_page_aligned(size_t value) noexcept {
    constexpr size_t page = 4096;
    if (value < page) value = page;
    return (value + page - 1) & ~(page - 1);
}

static bool adaptive_env_present(const char* name) noexcept {
    const char* env = std::getenv(name);
    return env && *env;
}

static void adaptive_init_runtime_config_once() noexcept {
    std::lock_guard<std::mutex> lock(g_adaptive_config_mutex);

    g_config_small_page_size.store(adaptive_page_aligned(
        static_cast<size_t>(adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_SMALL_PAGE_SIZE",
                                                   ADAPTIVE_SMALL_PAGE_SIZE,
                                                   16 * 1024,
                                                   1024 * 1024))),
        std::memory_order_relaxed);
    g_config_medium_span_size.store(adaptive_page_aligned(
        static_cast<size_t>(adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_MEDIUM_SPAN_SIZE",
                                                   ADAPTIVE_MEDIUM_SPAN_SIZE,
                                                   64 * 1024,
                                                   4 * 1024 * 1024))),
        std::memory_order_relaxed);
    g_config_empty_cache_limit.store(static_cast<uint32_t>(
        adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_EMPTY_CACHE_LIMIT",
                               ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT,
                               0,
                               128)),
        std::memory_order_relaxed);

    g_config_mode_window.store(static_cast<uint32_t>(
        adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_MODE_WINDOW",
                               ADAPTIVE_DEFAULT_MODE_WINDOW,
                               64,
                               1 << 20)),
        std::memory_order_relaxed);
    g_config_mode_cooldown.store(static_cast<uint32_t>(
        adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_MODE_COOLDOWN",
                               ADAPTIVE_DEFAULT_MODE_COOLDOWN,
                               0,
                               1024)),
        std::memory_order_relaxed);

    bool mode_auto = false;
    bool mode_env_present = adaptive_env_present("MY_MALLOC_ADAPTIVE_MODE");
    AdaptiveModeId configured = parse_mode(std::getenv("MY_MALLOC_ADAPTIVE_MODE"), &mode_auto);
    if (adaptive_env_present("MY_MALLOC_ADAPTIVE_DEBUG_MODE")) {
        const char* debug = std::getenv("MY_MALLOC_ADAPTIVE_DEBUG_MODE");
        if (adaptive_streq(debug, "1") || adaptive_streq(debug, "true")) {
            configured = AdaptiveModeId::HardenedDebug;
            mode_auto = false;
        }
    }
    AdaptiveModeSelectorKind selector = parse_mode_selector(std::getenv("MY_MALLOC_ADAPTIVE_MODE_SELECTOR"));
    if (mode_auto && selector == AdaptiveModeSelectorKind::Fixed) {
        selector = AdaptiveModeSelectorKind::Rule;
    } else if (!mode_auto && mode_env_present && selector == AdaptiveModeSelectorKind::Rule) {
        selector = AdaptiveModeSelectorKind::Fixed;
    }
    g_config_mode.store(static_cast<uint8_t>(configured), std::memory_order_relaxed);
    g_active_mode.store(static_cast<uint8_t>(configured), std::memory_order_relaxed);
    g_previous_mode.store(static_cast<uint8_t>(configured), std::memory_order_relaxed);
    g_mode_selector_kind.store(static_cast<uint8_t>(selector), std::memory_order_relaxed);
    for (size_t i = 0; i < ADAPTIVE_MODE_COUNT; ++i) {
        g_mode_states[i].store(static_cast<uint8_t>(AdaptiveModeState::Inactive), std::memory_order_relaxed);
    }
    g_mode_states[mode_index(configured)].store(static_cast<uint8_t>(AdaptiveModeState::Active), std::memory_order_relaxed);
}

static void adaptive_init_runtime_config() noexcept {
    pthread_once(&g_adaptive_config_once, adaptive_init_runtime_config_once);
}

static AdaptiveRuntimeConfig adaptive_runtime_config() noexcept {
    adaptive_init_runtime_config();
    AdaptiveRuntimeConfig cfg{};
    cfg.version = g_config_version.load(std::memory_order_relaxed);
    cfg.small_page_size = g_config_small_page_size.load(std::memory_order_relaxed);
    cfg.medium_span_size = g_config_medium_span_size.load(std::memory_order_relaxed);
    cfg.empty_cache_limit = g_config_empty_cache_limit.load(std::memory_order_relaxed);
    cfg.configured_mode = static_cast<AdaptiveModeId>(g_config_mode.load(std::memory_order_relaxed));
    cfg.mode_window = g_config_mode_window.load(std::memory_order_relaxed);
    cfg.mode_cooldown = g_config_mode_cooldown.load(std::memory_order_relaxed);
    return cfg;
}

// --- Storage helper selection ---------------------------------------------

static AdaptiveStorageId heuristic_storage(size_t size) noexcept {
    if (size <= 1024) return AdaptiveStorageId::PooledSmall;
    if (size <= 64 * 1024) return AdaptiveStorageId::PooledMedium;
    return AdaptiveStorageId::DirectMap;
}

static size_t storage_index(AdaptiveStorageId id) noexcept;
static bool adaptive_page_maybe_owned(void* ptr) noexcept;
static void register_adaptive_region(void* base, size_t size) noexcept;
static void unregister_adaptive_region(void* base, size_t size) noexcept;

static AdaptiveStorageId balanced_storage(size_t size) noexcept {
    return heuristic_storage(size);
}

static AdaptiveStorageId throughput_storage(size_t size) noexcept {
    if (size <= ADAPTIVE_SMALL_MAX) return AdaptiveStorageId::PooledSmall;
    if (size <= ADAPTIVE_MEDIUM_MAX) return AdaptiveStorageId::PooledMedium;
    return AdaptiveStorageId::DirectMap;
}

static AdaptiveStorageId deterministic_latency_storage(size_t size) noexcept {
    if (size <= ADAPTIVE_MEDIUM_MAX) return heuristic_storage(size);
    return AdaptiveStorageId::DirectMap;
}

static AdaptiveStorageId compact_rss_storage(size_t size) noexcept {
    return heuristic_storage(size);
}

static AdaptiveStorageId fragmentation_stable_storage(size_t size) noexcept {
    return heuristic_storage(size);
}

static AdaptiveStorageId cross_thread_storage(size_t size) noexcept {
    return heuristic_storage(size);
}

static AdaptiveStorageId large_object_streaming_storage(size_t size) noexcept {
    if (size > 4 * 1024) return AdaptiveStorageId::DirectMap;
    return AdaptiveStorageId::PooledSmall;
}

static AdaptiveStorageId hardened_debug_storage(size_t) noexcept {
    return AdaptiveStorageId::DirectMap;
}

static uint32_t mode_empty_keep_limit(AdaptiveModeId mode, uint32_t configured) noexcept {
    switch (mode) {
        case AdaptiveModeId::ThroughputCache:
            return configured < 8 ? 8 : configured;
        case AdaptiveModeId::DeterministicLatency:
            return configured < 4 ? 4 : configured;
        case AdaptiveModeId::CompactRSS:
        case AdaptiveModeId::LargeObjectStreaming:
        case AdaptiveModeId::HardenedDebug:
            return 0;
        case AdaptiveModeId::FragmentationStable:
            return configured < 1 ? 1 : configured;
        case AdaptiveModeId::CrossThreadMessage:
            return configured < 2 ? 2 : configured;
        case AdaptiveModeId::Balanced:
            return configured;
    }
    return configured;
}

// --- Storage helper index --------------------------------------------------

static size_t storage_index(AdaptiveStorageId id) noexcept {
    switch (id) {
        case AdaptiveStorageId::PooledSmall:  return 0;
        case AdaptiveStorageId::PooledMedium: return 1;
        case AdaptiveStorageId::DirectMap:  return 2;
    }
    return 0;
}

static const AdaptiveModeOps* mode_ops(AdaptiveModeId mode) noexcept;
static WorkloadSignature workload_signature_snapshot() noexcept;
static void* adaptive_mode_allocate(size_t size,
                                    AdaptiveModeId mode,
                                    AdaptiveStorageId* actual_storage) noexcept;
static void adaptive_mode_deallocate(AdaptiveHeader* hdr) noexcept;
static void* adaptive_mode_reallocate(void* ptr, size_t size) noexcept;
static size_t adaptive_mode_usable_size(AdaptiveHeader* hdr) noexcept;

static void mode_noop_activate(AdaptiveModeId) noexcept {}
static void mode_noop_retire(AdaptiveModeId) noexcept {}

// --- Header helpers --------------------------------------------------------

static void* user_from_header(AdaptiveHeader* hdr) noexcept {
    return reinterpret_cast<char*>(hdr) + ADAPTIVE_HDR_OFFSET;
}

static bool valid_storage_id(AdaptiveStorageId id) noexcept {
    return id == AdaptiveStorageId::PooledSmall ||
           id == AdaptiveStorageId::PooledMedium ||
           id == AdaptiveStorageId::DirectMap;
}

static bool header_is_plausible(AdaptiveHeader* hdr, void* user) noexcept {
    if (!hdr || hdr->magic != ADAPTIVE_MAGIC) return false;
    if (!valid_storage_id(hdr->storage)) return false;
    if (!valid_mode_id(hdr->mode_id)) return false;
    if (user_from_header(hdr) != user) return false;
    if (hdr->usable < hdr->requested) return false;
    if (hdr->flags & ADAPTIVE_FLAG_POOLED) {
        return hdr->owner_page != nullptr && hdr->region_base == hdr->owner_page;
    }
    return hdr->region_base != nullptr && hdr->mapped_size >= ADAPTIVE_HDR_OFFSET;
}

static AdaptiveHeader* header_from_user_fast(void* ptr) noexcept {
    if (!ptr) return nullptr;
    if (!adaptive_page_maybe_owned(ptr)) return nullptr;
    uintptr_t user = reinterpret_cast<uintptr_t>(ptr);
    if (user < ADAPTIVE_HDR_OFFSET) return nullptr;
    AdaptiveHeader* hdr = reinterpret_cast<AdaptiveHeader*>(user - ADAPTIVE_HDR_OFFSET);
    return header_is_plausible(hdr, ptr) ? hdr : nullptr;
}

static void registry_insert(AdaptiveHeader* hdr) noexcept {
    std::lock_guard<std::mutex> lock(g_adaptive_registry_mutex);
    hdr->registry_prev = nullptr;
    hdr->registry_next = g_adaptive_registry_head;
    if (g_adaptive_registry_head) {
        g_adaptive_registry_head->registry_prev = hdr;
    }
    g_adaptive_registry_head = hdr;
}

static void registry_remove(AdaptiveHeader* hdr) noexcept {
    std::lock_guard<std::mutex> lock(g_adaptive_registry_mutex);
    if (hdr->registry_prev) {
        hdr->registry_prev->registry_next = hdr->registry_next;
    } else if (g_adaptive_registry_head == hdr) {
        g_adaptive_registry_head = hdr->registry_next;
    }
    if (hdr->registry_next) {
        hdr->registry_next->registry_prev = hdr->registry_prev;
    }
    hdr->registry_prev = nullptr;
    hdr->registry_next = nullptr;
}

static AdaptiveHeader* registry_find(void* ptr) noexcept {
    if (AdaptiveHeader* hdr = header_from_user_fast(ptr)) return hdr;
    static bool debug_registry_lookup = []() noexcept {
        const char* env = std::getenv("MY_MALLOC_ADAPTIVE_DEBUG_REGISTRY");
        return adaptive_streq(env, "1") || adaptive_streq(env, "true");
    }();
    if (!debug_registry_lookup) return nullptr;
    std::lock_guard<std::mutex> lock(g_adaptive_registry_mutex);
    for (AdaptiveHeader* hdr = g_adaptive_registry_head; hdr; hdr = hdr->registry_next) {
        if (hdr->magic == ADAPTIVE_MAGIC && user_from_header(hdr) == ptr) {
            return hdr;
        }
    }
    return nullptr;
}

// --- Stats helpers ---------------------------------------------------------

static void record_alloc_stats(AdaptiveModeId mode, AdaptiveStorageId storage, size_t size,
                               size_t usable) noexcept {
    size_t idx = storage_index(storage);
    g_adaptive_stats.storage[idx].alloc_count.fetch_add(1, std::memory_order_relaxed);
    g_adaptive_stats.storage[idx].requested_bytes.fetch_add(size, std::memory_order_relaxed);
    g_adaptive_stats.storage[idx].usable_bytes.fetch_add(usable, std::memory_order_relaxed);
    g_size_histogram[size_histogram_index(size)].fetch_add(1, std::memory_order_relaxed);
    g_adaptive_stats.live_bytes.fetch_add(static_cast<int64_t>(usable), std::memory_order_relaxed);
    auto& mode_stats = g_adaptive_stats.mode[mode_index(mode)];
    mode_stats.alloc_count.fetch_add(1, std::memory_order_relaxed);
    mode_stats.live_bytes.fetch_add(static_cast<int64_t>(usable), std::memory_order_relaxed);
}

static void record_free_stats(AdaptiveHeader* hdr) noexcept {
    size_t idx = storage_index(hdr->storage);
    g_adaptive_stats.storage[idx].free_count.fetch_add(1, std::memory_order_relaxed);
    g_adaptive_stats.live_bytes.fetch_sub(static_cast<int64_t>(hdr->usable), std::memory_order_relaxed);
    auto& mode_stats = g_adaptive_stats.mode[mode_index(hdr->mode_id)];
    mode_stats.free_count.fetch_add(1, std::memory_order_relaxed);
    mode_stats.live_bytes.fetch_sub(static_cast<int64_t>(hdr->usable), std::memory_order_relaxed);
}

static void record_mapped_bytes(AdaptiveModeId mode, size_t mapped) noexcept {
    g_adaptive_stats.mapped_bytes.fetch_add(static_cast<int64_t>(mapped), std::memory_order_relaxed);
    g_adaptive_stats.mode[mode_index(mode)].mapped_bytes.fetch_add(static_cast<int64_t>(mapped), std::memory_order_relaxed);
}

static void record_unmapped_bytes(AdaptiveModeId mode, size_t mapped) noexcept {
    g_adaptive_stats.mapped_bytes.fetch_sub(static_cast<int64_t>(mapped), std::memory_order_relaxed);
    g_adaptive_stats.mode[mode_index(mode)].mapped_bytes.fetch_sub(static_cast<int64_t>(mapped), std::memory_order_relaxed);
}

static void record_pool_hit(AdaptiveStorageId storage) noexcept {
    g_adaptive_stats.storage[storage_index(storage)].pool_hits.fetch_add(1, std::memory_order_relaxed);
}

static void record_pool_miss(AdaptiveStorageId storage) noexcept {
    g_adaptive_stats.storage[storage_index(storage)].pool_misses.fetch_add(1, std::memory_order_relaxed);
}

// --- Core mmap allocation (shared by all v1 strategies) --------------------

static bool round_usable(size_t size, size_t& usable_out) noexcept {
    size_t usable = size;
    if (usable == 0) usable = 1;
    if (usable > static_cast<size_t>(-1) - 15) return false;
    usable_out = (usable + 15) & ~size_t(15);
    return true;
}

static size_t align_up(size_t value, size_t alignment) noexcept {
    return (value + alignment - 1) & ~(alignment - 1);
}

static uintptr_t page_key(uintptr_t addr) noexcept {
    return addr & ~uintptr_t(4095);
}

static size_t page_table_index(uintptr_t page) noexcept {
    return (page >> 12) & (ADAPTIVE_PAGE_TABLE_SIZE - 1);
}

static void register_adaptive_region(void* base, size_t size) noexcept {
    if (!base || size == 0) return;
    uintptr_t begin = page_key(reinterpret_cast<uintptr_t>(base));
    uintptr_t end = page_key(reinterpret_cast<uintptr_t>(base) + size - 1);
    for (uintptr_t page = begin; page <= end; page += 4096) {
        size_t idx = page_table_index(page);
        for (size_t probe = 0; probe < ADAPTIVE_PAGE_TABLE_PROBE; ++probe) {
            std::atomic<uintptr_t>& slot = g_adaptive_page_table[(idx + probe) & (ADAPTIVE_PAGE_TABLE_SIZE - 1)];
            uintptr_t expected = 0;
            if (slot.compare_exchange_strong(expected, page, std::memory_order_relaxed) ||
                expected == page) {
                break;
            }
            if (expected == ADAPTIVE_PAGE_TOMBSTONE) {
                expected = ADAPTIVE_PAGE_TOMBSTONE;
                if (slot.compare_exchange_strong(expected, page, std::memory_order_relaxed)) {
                    break;
                }
            }
        }
        if (page > static_cast<uintptr_t>(-1) - 4096) break;
    }
}

static void unregister_adaptive_region(void* base, size_t size) noexcept {
    if (!base || size == 0) return;
    uintptr_t begin = page_key(reinterpret_cast<uintptr_t>(base));
    uintptr_t end = page_key(reinterpret_cast<uintptr_t>(base) + size - 1);
    for (uintptr_t page = begin; page <= end; page += 4096) {
        size_t idx = page_table_index(page);
        for (size_t probe = 0; probe < ADAPTIVE_PAGE_TABLE_PROBE; ++probe) {
            std::atomic<uintptr_t>& slot = g_adaptive_page_table[(idx + probe) & (ADAPTIVE_PAGE_TABLE_SIZE - 1)];
            uintptr_t value = slot.load(std::memory_order_relaxed);
            if (value == page) {
                slot.store(ADAPTIVE_PAGE_TOMBSTONE, std::memory_order_relaxed);
                break;
            }
            if (value == 0) break;
        }
        if (page > static_cast<uintptr_t>(-1) - 4096) break;
    }
}

static bool adaptive_page_maybe_owned(void* ptr) noexcept {
    if (!ptr) return false;
    uintptr_t page = page_key(reinterpret_cast<uintptr_t>(ptr));
    size_t idx = page_table_index(page);
    for (size_t probe = 0; probe < ADAPTIVE_PAGE_TABLE_PROBE; ++probe) {
        uintptr_t value = g_adaptive_page_table[(idx + probe) & (ADAPTIVE_PAGE_TABLE_SIZE - 1)]
            .load(std::memory_order_relaxed);
        if (value == page) return true;
        if (value == 0) return false;
    }
    return false;
}

static bool small_class(size_t size, size_t& class_index, size_t& usable) noexcept {
    size_t rounded = size == 0 ? 1 : size;
    if (rounded > ADAPTIVE_SMALL_MAX) return false;
    rounded = align_up(rounded, ADAPTIVE_SMALL_CLASS_STEP);
    if (rounded == 0 || rounded > ADAPTIVE_SMALL_MAX) return false;
    class_index = (rounded / ADAPTIVE_SMALL_CLASS_STEP) - 1;
    usable = rounded;
    return true;
}

static bool medium_class(size_t size, size_t& class_index, size_t& usable) noexcept {
    size_t rounded = size == 0 ? 1 : size;
    if (rounded > ADAPTIVE_MEDIUM_MAX) return false;
    if (rounded <= ADAPTIVE_SMALL_MAX) rounded = ADAPTIVE_SMALL_MAX + 1;
    rounded = align_up(rounded, ADAPTIVE_MEDIUM_CLASS_STEP);
    if (rounded <= ADAPTIVE_SMALL_MAX || rounded > ADAPTIVE_MEDIUM_MAX) return false;
    class_index = ((rounded - ADAPTIVE_SMALL_MAX) / ADAPTIVE_MEDIUM_CLASS_STEP) - 1;
    usable = rounded;
    return class_index < ADAPTIVE_MEDIUM_CLASS_COUNT;
}

static AdaptivePage* create_pool_page(AdaptiveStorageId storage,
                                      AdaptiveModeId mode,
                                      size_t class_index,
                                      size_t usable,
                                      size_t mapped_size,
                                      uint32_t config_version) noexcept {
    size_t stride = align_up(ADAPTIVE_HDR_OFFSET + usable, 16);
    size_t first_block = align_up(sizeof(AdaptivePage), 16);
    if (stride == 0 || first_block >= mapped_size) return nullptr;

    size_t capacity = (mapped_size - first_block) / stride;
    if (capacity == 0 || capacity > static_cast<size_t>(UINT32_MAX)) return nullptr;

    void* region = mmap(nullptr, mapped_size, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) return nullptr;
    register_adaptive_region(region, mapped_size);

    AdaptivePage* page = static_cast<AdaptivePage*>(region);
    page->storage = storage;
    page->mode_id = mode;
    page->config_version = config_version;
    page->class_index = static_cast<uint16_t>(class_index);
    page->_pad = 0;
    page->block_usable = usable;
    page->block_stride = stride;
    page->mapped_size = mapped_size;
    page->capacity = static_cast<uint32_t>(capacity);
    page->live_count = 0;
    page->free_list = nullptr;
    page->next = nullptr;

    char* cursor = static_cast<char*>(region) + first_block;
    for (size_t i = 0; i < capacity; ++i) {
        AdaptiveHeader* hdr = reinterpret_cast<AdaptiveHeader*>(cursor + i * stride);
        hdr->magic = 0;
        hdr->storage = storage;
        hdr->mode_id = mode;
        hdr->flags = ADAPTIVE_FLAG_POOLED;
        hdr->_pad = 0;
        hdr->config_version = config_version;
        hdr->requested = 0;
        hdr->usable = usable;
        hdr->mapped_size = mapped_size;
        hdr->region_base = region;
        hdr->registry_prev = nullptr;
        hdr->registry_next = page->free_list;
        hdr->owner_page = page;
        hdr->owner_thread = 0;
        page->free_list = hdr;
    }

    record_mapped_bytes(mode, mapped_size);
    return page;
}

static AdaptivePage** pool_head_for(AdaptiveStorageId storage, size_t class_index) noexcept {
    if (storage == AdaptiveStorageId::PooledSmall) {
        return class_index < ADAPTIVE_SMALL_CLASS_COUNT ? &g_small_pages[class_index] : nullptr;
    }
    if (storage == AdaptiveStorageId::PooledMedium) {
        return class_index < ADAPTIVE_MEDIUM_CLASS_COUNT ? &g_medium_pages[class_index] : nullptr;
    }
    return nullptr;
}

static std::mutex* pool_mutex_for(AdaptiveStorageId storage, size_t class_index) noexcept {
    if (storage == AdaptiveStorageId::PooledSmall) {
        return class_index < ADAPTIVE_SMALL_CLASS_COUNT ? &g_small_class_mutexes[class_index] : nullptr;
    }
    if (storage == AdaptiveStorageId::PooledMedium) {
        return class_index < ADAPTIVE_MEDIUM_CLASS_COUNT ? &g_medium_class_mutexes[class_index] : nullptr;
    }
    return nullptr;
}

static uint32_t count_empty_pages_locked(AdaptivePage* head) noexcept {
    uint32_t count = 0;
    for (AdaptivePage* page = head; page; page = page->next) {
        if (page->live_count == 0) count++;
    }
    return count;
}

static void release_page_locked(AdaptivePage** head,
                                AdaptivePage* target,
                                AdaptivePage* prev) noexcept {
    if (!head || !target || target->live_count != 0) return;
    if (prev) {
        prev->next = target->next;
    } else {
        *head = target->next;
    }
    size_t mapped = target->mapped_size;
    AdaptiveStorageId storage = target->storage;
    AdaptiveModeId mode = target->mode_id;
    record_unmapped_bytes(mode, mapped);
    g_adaptive_stats.release_unmapped_bytes.fetch_add(mapped, std::memory_order_relaxed);
    if (storage == AdaptiveStorageId::PooledSmall) {
        g_adaptive_stats.released_pages.fetch_add(1, std::memory_order_relaxed);
    } else if (storage == AdaptiveStorageId::PooledMedium) {
        g_adaptive_stats.released_spans.fetch_add(1, std::memory_order_relaxed);
    }
    unregister_adaptive_region(target, mapped);
    munmap(target, mapped);
}

static void release_empty_pages_if_needed_locked(AdaptiveStorageId storage,
                                                 AdaptivePage** head,
                                                 uint32_t keep_limit) noexcept {
    if (!head) return;
    uint32_t empty = count_empty_pages_locked(*head);
    if (storage == AdaptiveStorageId::PooledSmall) {
        g_adaptive_stats.empty_pages.store(empty, std::memory_order_relaxed);
    } else if (storage == AdaptiveStorageId::PooledMedium) {
        g_adaptive_stats.empty_spans.store(empty, std::memory_order_relaxed);
    }
    while (empty > keep_limit) {
        AdaptivePage* prev = nullptr;
        AdaptivePage* page = *head;
        while (page && page->live_count != 0) {
            prev = page;
            page = page->next;
        }
        if (!page) break;
        release_page_locked(head, page, prev);
        empty--;
    }
}

static void* adaptive_pool_alloc(size_t size,
                                 AdaptiveModeId mode,
                                 AdaptiveStorageId storage) noexcept {
    size_t class_index = 0;
    size_t usable = 0;
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    size_t mapped_size = 0;
    if (storage == AdaptiveStorageId::PooledSmall) {
        if (!small_class(size, class_index, usable)) return nullptr;
        mapped_size = cfg.small_page_size;
    } else if (storage == AdaptiveStorageId::PooledMedium) {
        if (!medium_class(size, class_index, usable)) return nullptr;
        mapped_size = cfg.medium_span_size;
    } else {
        return nullptr;
    }

    AdaptiveHeader* hdr = nullptr;
    {
        AdaptivePage** head = pool_head_for(storage, class_index);
        std::mutex* mutex = pool_mutex_for(storage, class_index);
        if (!head || !mutex) return nullptr;
        std::lock_guard<std::mutex> lock(*mutex);

        AdaptivePage* page = *head;
        while (page && !page->free_list) {
            page = page->next;
        }
        if (!page) {
            record_pool_miss(storage);
            page = create_pool_page(storage, mode, class_index, usable, mapped_size, cfg.version);
            if (!page) return nullptr;
            page->next = *head;
            *head = page;
        } else {
            record_pool_hit(storage);
        }

        hdr = page->free_list;
        page->free_list = hdr->registry_next;
        page->live_count++;

        hdr->magic = ADAPTIVE_MAGIC;
        hdr->storage = storage;
        hdr->mode_id = mode;
        hdr->flags = ADAPTIVE_FLAG_POOLED;
        hdr->_pad = 0;
        hdr->config_version = page->config_version;
        hdr->requested = size;
        hdr->usable = usable;
        hdr->mapped_size = page->mapped_size;
        hdr->region_base = page;
        hdr->registry_prev = nullptr;
        hdr->registry_next = nullptr;
        hdr->owner_page = page;
        hdr->owner_thread = current_thread_token();
    }

    registry_insert(hdr);
    record_alloc_stats(mode, storage, size, usable);
    return user_from_header(hdr);
}

static void adaptive_pool_free(AdaptiveHeader* hdr) noexcept {
    registry_remove(hdr);
    if (hdr->owner_thread == current_thread_token()) {
        g_adaptive_stats.same_thread_free_count.fetch_add(1, std::memory_order_relaxed);
    } else {
        g_adaptive_stats.remote_free_count.fetch_add(1, std::memory_order_relaxed);
    }
    record_free_stats(hdr);

    AdaptivePage* page = hdr->owner_page;
    if (!page) return;
    AdaptiveStorageId storage = page->storage;
    size_t class_index = page->class_index;
    AdaptivePage** head = pool_head_for(storage, class_index);
    std::mutex* mutex = pool_mutex_for(storage, class_index);
    if (!head || !mutex) return;
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    std::lock_guard<std::mutex> lock(*mutex);
    if (hdr->mode_id == AdaptiveModeId::HardenedDebug && hdr->usable > 0) {
        std::memset(user_from_header(hdr), 0xDD, hdr->usable);
    }
    hdr->magic = 0;
    hdr->requested = 0;
    hdr->owner_thread = 0;
    hdr->registry_prev = nullptr;
    hdr->registry_next = page->free_list;
    page->free_list = hdr;
    if (page->live_count > 0) {
        page->live_count--;
    }
    release_empty_pages_if_needed_locked(storage, head,
                                         mode_empty_keep_limit(hdr->mode_id, cfg.empty_cache_limit));
}

static void* adaptive_mmap_alloc(size_t size,
                                 AdaptiveModeId mode,
                                 AdaptiveStorageId storage) noexcept {
    size_t usable = 0;
    if (!round_usable(size, usable)) return nullptr;
    if (usable > static_cast<size_t>(-1) - ADAPTIVE_HDR_OFFSET) return nullptr;
    size_t total = ADAPTIVE_HDR_OFFSET + usable;
    constexpr size_t page_size = 4096;
    if (total > static_cast<size_t>(-1) - (page_size - 1)) return nullptr;
    size_t mapped = (total + page_size - 1) & ~(page_size - 1);

    void* region = mmap(nullptr, mapped, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) return nullptr;
    register_adaptive_region(region, mapped);

    AdaptiveHeader* hdr = static_cast<AdaptiveHeader*>(region);
    hdr->magic       = ADAPTIVE_MAGIC;
    hdr->storage    = storage;
    hdr->mode_id     = mode;
    hdr->flags       = 0;
    hdr->_pad        = 0;
    hdr->config_version = adaptive_runtime_config().version;
    hdr->requested   = size;
    hdr->usable      = usable;
    hdr->mapped_size = mapped;
    hdr->region_base = region;
    hdr->registry_prev = nullptr;
    hdr->registry_next = nullptr;
    hdr->owner_page = nullptr;
    hdr->owner_thread = current_thread_token();

    g_adaptive_stats.mmap_count.fetch_add(1, std::memory_order_relaxed);
    record_mapped_bytes(mode, mapped);
    registry_insert(hdr);
    record_alloc_stats(mode, storage, size, usable);
    return user_from_header(hdr);
}

static void adaptive_mmap_free(AdaptiveHeader* hdr) noexcept {
    registry_remove(hdr);
    if (hdr->owner_thread == current_thread_token()) {
        g_adaptive_stats.same_thread_free_count.fetch_add(1, std::memory_order_relaxed);
    } else {
        g_adaptive_stats.remote_free_count.fetch_add(1, std::memory_order_relaxed);
    }
    record_free_stats(hdr);
    void* base = hdr->region_base;
    size_t mapped = hdr->mapped_size;
    AdaptiveModeId mode = hdr->mode_id;
    if (mode == AdaptiveModeId::HardenedDebug && hdr->usable > 0) {
        std::memset(user_from_header(hdr), 0xDD, hdr->usable);
        hdr->magic = 0;
        // Phase-1 hardened quarantine: keep the mapping and ownership-table
        // entry so double frees route back to adaptive diagnostics instead of
        // falling through to the ptmalloc teaching path with an unmapped ptr.
        return;
    }
    g_adaptive_stats.munmap_count.fetch_add(1, std::memory_order_relaxed);
    record_unmapped_bytes(mode, mapped);
    unregister_adaptive_region(base, mapped);
    munmap(base, mapped);
}

static void* allocate_with_storage(size_t size,
                                    AdaptiveModeId mode,
                                    AdaptiveStorageId storage) noexcept {
    if (storage == AdaptiveStorageId::PooledSmall ||
        storage == AdaptiveStorageId::PooledMedium) {
        return adaptive_pool_alloc(size, mode, storage);
    }
    return adaptive_mmap_alloc(size, mode, AdaptiveStorageId::DirectMap);
}

static void* adaptive_mode_allocate(size_t size,
                                    AdaptiveModeId mode,
                                    AdaptiveStorageId* actual_storage) noexcept {
    const AdaptiveModeOps* ops = mode_ops(mode);
    AdaptiveStorageId storage = ops->select_storage(size);
    if (actual_storage) *actual_storage = storage;
    void* ptr = allocate_with_storage(size, mode, storage);
    if (!ptr && storage != heuristic_storage(size)) {
        storage = heuristic_storage(size);
        if (actual_storage) *actual_storage = storage;
        ptr = allocate_with_storage(size, mode, storage);
    }
    return ptr;
}

static void adaptive_mode_deallocate(AdaptiveHeader* hdr) noexcept {
    if (hdr->flags & ADAPTIVE_FLAG_POOLED) {
        adaptive_pool_free(hdr);
    } else {
        adaptive_mmap_free(hdr);
    }
}

static void* adaptive_mode_reallocate(void* ptr, size_t size) noexcept {
    AdaptiveHeader* hdr = registry_find(ptr);
    if (!hdr) return nullptr;
    size_t old_usable = hdr->usable;
    if (size <= old_usable) return ptr;

    AdaptiveModeId mode = hdr->mode_id;
    AdaptiveStorageId actual = heuristic_storage(size);
    void* new_ptr = adaptive_mode_allocate(size, mode, &actual);
    if (!new_ptr) return nullptr;
    size_t copy = old_usable < size ? old_usable : size;
    std::memcpy(new_ptr, ptr, copy);
    adaptive_mode_deallocate(hdr);
    return new_ptr;
}

static size_t adaptive_mode_usable_size(AdaptiveHeader* hdr) noexcept {
    return hdr ? hdr->usable : 0;
}

static const AdaptiveModeOps g_mode_table[ADAPTIVE_MODE_COUNT] = {
    {AdaptiveModeId::Balanced, "balanced",
     "Default stable baseline for unknown or mixed workloads.",
     balanced_storage, adaptive_mode_allocate, adaptive_mode_deallocate,
     adaptive_mode_reallocate, adaptive_mode_usable_size, mode_noop_activate, mode_noop_retire},
    {AdaptiveModeId::ThroughputCache, "throughput_cache",
     "High-throughput local malloc/free with cache-heavy reuse and higher RSS tolerance.",
     throughput_storage, adaptive_mode_allocate, adaptive_mode_deallocate,
     adaptive_mode_reallocate, adaptive_mode_usable_size, mode_noop_activate, mode_noop_retire},
    {AdaptiveModeId::DeterministicLatency, "deterministic_latency",
     "Low jitter baseline that avoids aggressive release on hot pooled paths.",
     deterministic_latency_storage, adaptive_mode_allocate, adaptive_mode_deallocate,
     adaptive_mode_reallocate, adaptive_mode_usable_size, mode_noop_activate, mode_noop_retire},
    {AdaptiveModeId::CompactRSS, "compact_rss",
     "Memory-constrained mode with aggressive empty page/span release.",
     compact_rss_storage, adaptive_mode_allocate, adaptive_mode_deallocate,
     adaptive_mode_reallocate, adaptive_mode_usable_size, mode_noop_activate, mode_noop_retire},
    {AdaptiveModeId::FragmentationStable, "fragmentation_stable",
     "Long-running mixed-size mode focused on mapped/live and usable/requested stability.",
     fragmentation_stable_storage, adaptive_mode_allocate, adaptive_mode_deallocate,
     adaptive_mode_reallocate, adaptive_mode_usable_size, mode_noop_activate, mode_noop_retire},
    {AdaptiveModeId::CrossThreadMessage, "cross_thread",
     "Producer-consumer mode with owner-thread telemetry and remote-free hooks.",
     cross_thread_storage, adaptive_mode_allocate, adaptive_mode_deallocate,
     adaptive_mode_reallocate, adaptive_mode_usable_size, mode_noop_activate, mode_noop_retire},
    {AdaptiveModeId::LargeObjectStreaming, "large_object",
     "Large object streaming mode that favors direct mmap and fast OS return.",
     large_object_streaming_storage, adaptive_mode_allocate, adaptive_mode_deallocate,
     adaptive_mode_reallocate, adaptive_mode_usable_size, mode_noop_activate, mode_noop_retire},
    {AdaptiveModeId::HardenedDebug, "hardened_debug",
     "Safety/debug mode with stronger validation, diagnostics, and poison-on-free.",
     hardened_debug_storage, adaptive_mode_allocate, adaptive_mode_deallocate,
     adaptive_mode_reallocate, adaptive_mode_usable_size, mode_noop_activate, mode_noop_retire},
};

static const AdaptiveModeOps* mode_ops(AdaptiveModeId mode) noexcept {
    return &g_mode_table[mode_index(mode)];
}

static double ratio_u64(uint64_t num, uint64_t den) noexcept {
    return den == 0 ? 0.0 : static_cast<double>(num) / static_cast<double>(den);
}

static WorkloadSignature workload_signature_snapshot() noexcept {
    WorkloadSignature w{};
    w.alloc_calls = g_adaptive_stats.malloc_calls.load(std::memory_order_relaxed);
    w.free_calls = g_adaptive_stats.free_calls.load(std::memory_order_relaxed);
    w.remote_free_count = g_adaptive_stats.remote_free_count.load(std::memory_order_relaxed);
    w.same_thread_free_count = g_adaptive_stats.same_thread_free_count.load(std::memory_order_relaxed);
    w.mmap_count = g_adaptive_stats.mmap_count.load(std::memory_order_relaxed);
    w.munmap_count = g_adaptive_stats.munmap_count.load(std::memory_order_relaxed);
    w.slow_path_count = g_adaptive_stats.slow_path_count.load(std::memory_order_relaxed);
    w.invalid_free_count = g_adaptive_stats.invalid_free_count.load(std::memory_order_relaxed);
    w.double_free_count = g_adaptive_stats.double_free_count.load(std::memory_order_relaxed);
    w.live_bytes = g_adaptive_stats.live_bytes.load(std::memory_order_relaxed);
    w.mapped_bytes = g_adaptive_stats.mapped_bytes.load(std::memory_order_relaxed);
    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        w.requested_bytes += g_adaptive_stats.storage[i].requested_bytes.load(std::memory_order_relaxed);
        w.usable_bytes += g_adaptive_stats.storage[i].usable_bytes.load(std::memory_order_relaxed);
        w.pool_hits += g_adaptive_stats.storage[i].pool_hits.load(std::memory_order_relaxed);
        w.pool_misses += g_adaptive_stats.storage[i].pool_misses.load(std::memory_order_relaxed);
    }
    w.large_bytes = g_adaptive_stats.storage[storage_index(AdaptiveStorageId::DirectMap)]
        .requested_bytes.load(std::memory_order_relaxed);
    uint64_t hist_total = 0;
    for (size_t i = 0; i < 8; ++i) {
        w.size_hist[i] = g_size_histogram[i].load(std::memory_order_relaxed);
        hist_total += w.size_hist[i];
    }
    if (hist_total > 0) {
        for (size_t i = 0; i < 8; ++i) {
            if (w.size_hist[i] == 0) continue;
            double p = static_cast<double>(w.size_hist[i]) / static_cast<double>(hist_total);
            w.size_entropy -= p * (std::log(p) / std::log(2.0));
        }
    }
    w.large_bytes_ratio = ratio_u64(w.large_bytes, w.requested_bytes);
    w.mapped_live_ratio = w.live_bytes > 0
        ? static_cast<double>(w.mapped_bytes) / static_cast<double>(w.live_bytes)
        : 0.0;
    w.fragmentation_estimate = w.usable_bytes > w.requested_bytes && w.requested_bytes > 0
        ? static_cast<double>(w.usable_bytes - w.requested_bytes) / static_cast<double>(w.requested_bytes)
        : 0.0;
    uint64_t thread_frees = w.remote_free_count + w.same_thread_free_count;
    w.remote_free_ratio = ratio_u64(w.remote_free_count, thread_frees);
    w.slow_path_ratio = ratio_u64(w.mmap_count + w.pool_misses + w.slow_path_count, w.alloc_calls);
    w.reuse_rate = ratio_u64(w.pool_hits, w.pool_hits + w.pool_misses);
    return w;
}

static AdaptiveModeId rule_select_mode(const WorkloadSignature& w) noexcept {
    const char* debug = std::getenv("MY_MALLOC_ADAPTIVE_DEBUG_MODE");
    if (adaptive_streq(debug, "1") || adaptive_streq(debug, "true")) {
        return AdaptiveModeId::HardenedDebug;
    }
    if (w.mapped_live_ratio > 8.0 && w.mapped_bytes > 4 * 1024 * 1024) {
        return AdaptiveModeId::CompactRSS;
    }
    if (w.remote_free_ratio > 0.20 && (w.remote_free_count + w.same_thread_free_count) > 64) {
        return AdaptiveModeId::CrossThreadMessage;
    }
    if (w.large_bytes_ratio > 0.50 && w.requested_bytes > 1024 * 1024) {
        return AdaptiveModeId::LargeObjectStreaming;
    }
    if (w.slow_path_ratio > 0.35 && w.alloc_calls > 256) {
        return AdaptiveModeId::DeterministicLatency;
    }
    if (w.size_entropy > 2.2 && w.fragmentation_estimate > 0.20) {
        return AdaptiveModeId::FragmentationStable;
    }
    if (w.reuse_rate > 0.70 && w.remote_free_ratio < 0.05 && w.alloc_calls > 512) {
        return AdaptiveModeId::ThroughputCache;
    }
    return AdaptiveModeId::Balanced;
}

static void activate_mode(AdaptiveModeId next) noexcept {
    AdaptiveModeId old = static_cast<AdaptiveModeId>(g_active_mode.load(std::memory_order_relaxed));
    if (old == next) return;
    mode_ops(old)->on_retire(old);
    g_mode_states[mode_index(old)].store(static_cast<uint8_t>(AdaptiveModeState::Retired),
                                         std::memory_order_relaxed);
    g_previous_mode.store(static_cast<uint8_t>(old), std::memory_order_relaxed);
    g_active_mode.store(static_cast<uint8_t>(next), std::memory_order_relaxed);
    g_mode_states[mode_index(next)].store(static_cast<uint8_t>(AdaptiveModeState::Active),
                                          std::memory_order_relaxed);
    g_adaptive_stats.mode_switches.fetch_add(1, std::memory_order_relaxed);
    mode_ops(next)->on_activate(next);
}

static AdaptiveModeId select_active_mode_for_malloc() noexcept {
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    AdaptiveModeId current = static_cast<AdaptiveModeId>(g_active_mode.load(std::memory_order_relaxed));
    AdaptiveModeSelectorKind selector = static_cast<AdaptiveModeSelectorKind>(
        g_mode_selector_kind.load(std::memory_order_relaxed));
    if (selector == AdaptiveModeSelectorKind::Fixed ||
        selector == AdaptiveModeSelectorKind::Manual) {
        return current;
    }

    uint64_t n = g_mode_decision_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    uint64_t cooldown_until = g_mode_cooldown_until.load(std::memory_order_relaxed);
    if (n < cooldown_until || (n % cfg.mode_window) != 0) return current;

    WorkloadSignature w = workload_signature_snapshot();
    AdaptiveModeId next = rule_select_mode(w);
    if (next == current) return current;

    bool severe_memory_pressure = w.mapped_live_ratio > 12.0 && w.mapped_bytes > 8 * 1024 * 1024;
    bool debug_forced = next == AdaptiveModeId::HardenedDebug;
    bool expected_gain = severe_memory_pressure || debug_forced ||
        w.remote_free_ratio > 0.25 ||
        w.large_bytes_ratio > 0.60 ||
        w.slow_path_ratio > 0.45 ||
        w.fragmentation_estimate > 0.30 ||
        w.reuse_rate > 0.80;
    if (!expected_gain) return current;

    activate_mode(next);
    uint64_t next_allowed = n + static_cast<uint64_t>(cfg.mode_cooldown) *
        static_cast<uint64_t>(cfg.mode_window);
    g_mode_cooldown_until.store(next_allowed, std::memory_order_relaxed);
    return next;
}

// --- Public API ------------------------------------------------------------

void* adaptive_malloc(size_t size) noexcept {
    g_adaptive_stats.malloc_calls.fetch_add(1, std::memory_order_relaxed);

    AdaptiveModeId mode = select_active_mode_for_malloc();
    AdaptiveStorageId actual_storage = heuristic_storage(size);
    void* ptr = mode_ops(mode)->allocate(size, mode, &actual_storage);
    (void)actual_storage;
    if (!ptr) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
    }
    return ptr;
}

void adaptive_free(void* ptr) noexcept {
    if (!ptr) return;
    g_adaptive_stats.free_calls.fetch_add(1, std::memory_order_relaxed);

    AdaptiveHeader* hdr = registry_find(ptr);
    if (!hdr) {
        if (adaptive_page_maybe_owned(ptr)) {
            uintptr_t user = reinterpret_cast<uintptr_t>(ptr);
            if (user >= ADAPTIVE_HDR_OFFSET) {
                AdaptiveHeader* candidate = reinterpret_cast<AdaptiveHeader*>(user - ADAPTIVE_HDR_OFFSET);
                if (adaptive_page_maybe_owned(candidate) && candidate->magic == 0) {
                    g_adaptive_stats.double_free_count.fetch_add(1, std::memory_order_relaxed);
                } else if (adaptive_page_maybe_owned(candidate)) {
                    g_adaptive_stats.header_corruption_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        }
        g_adaptive_stats.invalid_free_count.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    mode_ops(hdr->mode_id)->deallocate(hdr);
}

void* adaptive_realloc(void* ptr, size_t size) noexcept {
    g_adaptive_stats.realloc_calls.fetch_add(1, std::memory_order_relaxed);

    if (!ptr) return adaptive_malloc(size);

    if (size == 0) {
        adaptive_free(ptr);
        return nullptr;
    }

    AdaptiveHeader* hdr = registry_find(ptr);
    if (!hdr) {
        g_adaptive_stats.invalid_free_count.fetch_add(1, std::memory_order_relaxed);
        return nullptr;
    }

    return mode_ops(hdr->mode_id)->reallocate(ptr, size);
}

size_t adaptive_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;
    AdaptiveHeader* hdr = registry_find(ptr);
    if (!hdr) return 0;
    return mode_ops(hdr->mode_id)->usable_size(hdr);
}

void* adaptive_memalign(size_t alignment, size_t size) noexcept {
    if (alignment < sizeof(void*)) alignment = sizeof(void*);
    if (alignment <= 16) {
        return adaptive_malloc(size);
    }

    // Ensure alignment is power of 2
    if (alignment & (alignment - 1)) {
        alignment--;
        alignment |= alignment >> 1;
        alignment |= alignment >> 2;
        alignment |= alignment >> 4;
        alignment |= alignment >> 8;
        alignment |= alignment >> 16;
        alignment |= alignment >> 32;
        alignment++;
        if (alignment == 0) {
            g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
            return nullptr;
        }
    }

    g_adaptive_stats.malloc_calls.fetch_add(1, std::memory_order_relaxed);
    AdaptiveModeId mode = select_active_mode_for_malloc();
    AdaptiveStorageId storage = mode_ops(mode)->select_storage(size);

    size_t usable = 0;
    if (!round_usable(size, usable)) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
        return nullptr;
    }
    // Over-allocate: header + usable + alignment for shifting
    if (usable > static_cast<size_t>(-1) - ADAPTIVE_HDR_OFFSET ||
        alignment > static_cast<size_t>(-1) - ADAPTIVE_HDR_OFFSET - usable) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
        return nullptr;
    }
    size_t total = ADAPTIVE_HDR_OFFSET + usable + alignment;
    constexpr size_t page_size = 4096;
    if (total > static_cast<size_t>(-1) - (page_size - 1)) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
        return nullptr;
    }
    size_t mapped = (total + page_size - 1) & ~(page_size - 1);

    void* region = mmap(nullptr, mapped, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
        return nullptr;
    }
    register_adaptive_region(region, mapped);

    // Find aligned user pointer
    uintptr_t base_user = reinterpret_cast<uintptr_t>(region) + ADAPTIVE_HDR_OFFSET;
    uintptr_t aligned_user = (base_user + alignment - 1) & ~(alignment - 1);

    // Place header just before aligned user pointer
    AdaptiveHeader* hdr = reinterpret_cast<AdaptiveHeader*>(
        aligned_user - ADAPTIVE_HDR_OFFSET);

    hdr->magic       = ADAPTIVE_MAGIC;
    hdr->storage    = storage;
    hdr->mode_id     = mode;
    hdr->flags       = ADAPTIVE_FLAG_MEMALIGN;
    hdr->_pad        = 0;
    hdr->config_version = adaptive_runtime_config().version;
    hdr->requested   = size;
    hdr->usable      = usable;
    hdr->mapped_size = mapped;
    hdr->region_base = region;  // actual mmap base for munmap
    hdr->registry_prev = nullptr;
    hdr->registry_next = nullptr;
    hdr->owner_page = nullptr;
    hdr->owner_thread = current_thread_token();

    g_adaptive_stats.mmap_count.fetch_add(1, std::memory_order_relaxed);
    record_mapped_bytes(mode, mapped);
    registry_insert(hdr);
    record_alloc_stats(mode, storage, size, usable);
    return reinterpret_cast<void*>(aligned_user);
}

bool adaptive_owns(void* ptr) noexcept {
    if (!ptr) return false;
    return registry_find(ptr) != nullptr || adaptive_page_maybe_owned(ptr);
}

// --- Stats -----------------------------------------------------------------

AdaptiveStatsSnapshot adaptive_stats_snapshot() noexcept {
    AdaptiveStatsSnapshot s;
    s.malloc_calls     = g_adaptive_stats.malloc_calls.load(std::memory_order_relaxed);
    s.free_calls       = g_adaptive_stats.free_calls.load(std::memory_order_relaxed);
    s.realloc_calls    = g_adaptive_stats.realloc_calls.load(std::memory_order_relaxed);
    s.failure_count    = g_adaptive_stats.failure_count.load(std::memory_order_relaxed);
    s.empty_pages = g_adaptive_stats.empty_pages.load(std::memory_order_relaxed);
    s.empty_spans = g_adaptive_stats.empty_spans.load(std::memory_order_relaxed);
    s.released_pages = g_adaptive_stats.released_pages.load(std::memory_order_relaxed);
    s.released_spans = g_adaptive_stats.released_spans.load(std::memory_order_relaxed);
    s.release_unmapped_bytes = g_adaptive_stats.release_unmapped_bytes.load(std::memory_order_relaxed);
    s.current_mode = static_cast<AdaptiveModeId>(g_active_mode.load(std::memory_order_relaxed));
    s.active_mode = s.current_mode;
    s.previous_mode = static_cast<AdaptiveModeId>(g_previous_mode.load(std::memory_order_relaxed));
    s.mode_switches = g_adaptive_stats.mode_switches.load(std::memory_order_relaxed);
    s.retired_mode_count = 0;
    for (size_t i = 0; i < ADAPTIVE_MODE_COUNT; ++i) {
        s.mode_alloc_count[i] = g_adaptive_stats.mode[i].alloc_count.load(std::memory_order_relaxed);
        s.mode_free_count[i] = g_adaptive_stats.mode[i].free_count.load(std::memory_order_relaxed);
        s.mode_live_bytes[i] = g_adaptive_stats.mode[i].live_bytes.load(std::memory_order_relaxed);
        s.mode_mapped_bytes[i] = g_adaptive_stats.mode[i].mapped_bytes.load(std::memory_order_relaxed);
        if (g_mode_states[i].load(std::memory_order_relaxed) ==
            static_cast<uint8_t>(AdaptiveModeState::Retired)) {
            s.retired_mode_count++;
        }
    }

    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        s.storage[i].alloc_count     = g_adaptive_stats.storage[i].alloc_count.load(std::memory_order_relaxed);
        s.storage[i].free_count      = g_adaptive_stats.storage[i].free_count.load(std::memory_order_relaxed);
        s.storage[i].requested_bytes = g_adaptive_stats.storage[i].requested_bytes.load(std::memory_order_relaxed);
        s.storage[i].usable_bytes    = g_adaptive_stats.storage[i].usable_bytes.load(std::memory_order_relaxed);
        s.storage[i].pool_hits       = g_adaptive_stats.storage[i].pool_hits.load(std::memory_order_relaxed);
        s.storage[i].pool_misses     = g_adaptive_stats.storage[i].pool_misses.load(std::memory_order_relaxed);
    }

    s.live_bytes   = g_adaptive_stats.live_bytes.load(std::memory_order_relaxed);
    s.mapped_bytes = g_adaptive_stats.mapped_bytes.load(std::memory_order_relaxed);
    s.mapped_live_ratio = s.live_bytes > 0
        ? static_cast<double>(s.mapped_bytes) / static_cast<double>(s.live_bytes)
        : 0.0;
    WorkloadSignature w = workload_signature_snapshot();
    s.remote_free_ratio = w.remote_free_ratio;
    s.size_entropy = w.size_entropy;
    s.large_bytes_ratio = w.large_bytes_ratio;
    s.fragmentation_estimate = w.fragmentation_estimate;
    s.slow_path_ratio = w.slow_path_ratio;
    s.double_free_count = g_adaptive_stats.double_free_count.load(std::memory_order_relaxed);
    s.invalid_free_count = g_adaptive_stats.invalid_free_count.load(std::memory_order_relaxed);
    s.header_corruption_count = g_adaptive_stats.header_corruption_count.load(std::memory_order_relaxed);
    return s;
}

void adaptive_stats_reset() noexcept {
    g_adaptive_stats.malloc_calls.store(0, std::memory_order_relaxed);
    g_adaptive_stats.free_calls.store(0, std::memory_order_relaxed);
    g_adaptive_stats.realloc_calls.store(0, std::memory_order_relaxed);
    g_adaptive_stats.failure_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.empty_pages.store(0, std::memory_order_relaxed);
    g_adaptive_stats.empty_spans.store(0, std::memory_order_relaxed);
    g_adaptive_stats.released_pages.store(0, std::memory_order_relaxed);
    g_adaptive_stats.released_spans.store(0, std::memory_order_relaxed);
    g_adaptive_stats.release_unmapped_bytes.store(0, std::memory_order_relaxed);
    g_adaptive_stats.mode_switches.store(0, std::memory_order_relaxed);
    g_adaptive_stats.remote_free_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.same_thread_free_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.invalid_free_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.double_free_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.header_corruption_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.slow_path_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.mmap_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.munmap_count.store(0, std::memory_order_relaxed);
    for (size_t i = 0; i < AdaptiveStats::NUM_STORAGE_HELPERS; ++i) {
        g_adaptive_stats.storage[i].alloc_count.store(0, std::memory_order_relaxed);
        g_adaptive_stats.storage[i].free_count.store(0, std::memory_order_relaxed);
        g_adaptive_stats.storage[i].requested_bytes.store(0, std::memory_order_relaxed);
        g_adaptive_stats.storage[i].usable_bytes.store(0, std::memory_order_relaxed);
        g_adaptive_stats.storage[i].pool_hits.store(0, std::memory_order_relaxed);
        g_adaptive_stats.storage[i].pool_misses.store(0, std::memory_order_relaxed);
    }
    g_adaptive_stats.live_bytes.store(0, std::memory_order_relaxed);
    g_adaptive_stats.mapped_bytes.store(0, std::memory_order_relaxed);
    for (size_t i = 0; i < ADAPTIVE_MODE_COUNT; ++i) {
        g_adaptive_stats.mode[i].alloc_count.store(0, std::memory_order_relaxed);
        g_adaptive_stats.mode[i].free_count.store(0, std::memory_order_relaxed);
        g_adaptive_stats.mode[i].live_bytes.store(0, std::memory_order_relaxed);
        g_adaptive_stats.mode[i].mapped_bytes.store(0, std::memory_order_relaxed);
    }
    for (size_t i = 0; i < 8; ++i) {
        g_size_histogram[i].store(0, std::memory_order_relaxed);
    }
    g_mode_decision_counter.store(0, std::memory_order_relaxed);
    g_mode_cooldown_until.store(0, std::memory_order_relaxed);
}

AdaptiveModeId adaptive_current_mode() noexcept {
    adaptive_init_runtime_config();
    return static_cast<AdaptiveModeId>(g_active_mode.load(std::memory_order_relaxed));
}

const char* adaptive_mode_name(AdaptiveModeId m) noexcept {
    return mode_ops(m)->name;
}

const char* adaptive_mode_objective(AdaptiveModeId m) noexcept {
    return mode_ops(m)->objective;
}

void adaptive_set_mode(AdaptiveModeId mode) noexcept {
    if (!valid_mode_id(mode)) return;
    adaptive_init_runtime_config();
    activate_mode(mode);
}

} // namespace my_ptmalloc
