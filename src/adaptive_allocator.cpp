// Independent adaptive allocator backend.
// All allocations carry an AdaptiveHeader for ownership and strategy tracking.
// V2: small/medium use adaptive-owned pages/spans and size-class free lists;
// large/aligned allocations use direct mmap.

#include "my_ptmalloc/adaptive_allocator.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <pthread.h>
#include <sys/mman.h>

namespace my_ptmalloc {

// --- Global state ----------------------------------------------------------

static AdaptiveStats g_adaptive_stats;
static pthread_once_t g_adaptive_policy_once = PTHREAD_ONCE_INIT;
static AdaptiveInternalPolicy g_adaptive_policy = AdaptiveInternalPolicy::Heuristic;
static pthread_once_t g_adaptive_parameter_policy_once = PTHREAD_ONCE_INIT;
static pthread_once_t g_adaptive_config_once = PTHREAD_ONCE_INIT;
static AdaptiveParameterPolicy g_adaptive_parameter_policy = AdaptiveParameterPolicy::Heuristic;
static std::atomic<uint64_t> g_adaptive_rr_counter{0};
static std::mutex g_adaptive_registry_mutex;
static AdaptiveHeader* g_adaptive_registry_head = nullptr;

struct AdaptivePage {
    AdaptiveStrategyId strategy;
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
constexpr uint32_t ADAPTIVE_DEFAULT_ARCH_WINDOW = 4096;
constexpr uint32_t ADAPTIVE_DEFAULT_PARAM_WINDOW = 8192;
constexpr uint32_t ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT = 2;
constexpr uint32_t ADAPTIVE_DEFAULT_COOLDOWN_WINDOWS = 2;
constexpr size_t ADAPTIVE_PAGE_TABLE_SIZE = 262144;
constexpr size_t ADAPTIVE_PAGE_TABLE_PROBE = 16;
constexpr uintptr_t ADAPTIVE_PAGE_TOMBSTONE = static_cast<uintptr_t>(-1);

struct AdaptiveRuntimeConfig {
    uint32_t version;
    size_t small_page_size;
    size_t medium_span_size;
    uint32_t architecture_window;
    uint32_t parameter_window;
    uint32_t local_batch_size;
    uint32_t empty_cache_limit;
    uint32_t cooldown_windows;
    AdaptiveProfileId profile;
};

std::mutex g_adaptive_config_mutex;
std::mutex g_small_class_mutexes[ADAPTIVE_SMALL_CLASS_COUNT];
std::mutex g_medium_class_mutexes[ADAPTIVE_MEDIUM_CLASS_COUNT];
AdaptivePage* g_small_pages[ADAPTIVE_SMALL_CLASS_COUNT]{};
AdaptivePage* g_medium_pages[ADAPTIVE_MEDIUM_CLASS_COUNT]{};
std::atomic<uint32_t> g_config_version{1};
std::atomic<size_t> g_config_small_page_size{ADAPTIVE_SMALL_PAGE_SIZE};
std::atomic<size_t> g_config_medium_span_size{ADAPTIVE_MEDIUM_SPAN_SIZE};
std::atomic<uint32_t> g_config_architecture_window{ADAPTIVE_DEFAULT_ARCH_WINDOW};
std::atomic<uint32_t> g_config_parameter_window{ADAPTIVE_DEFAULT_PARAM_WINDOW};
std::atomic<uint32_t> g_config_local_batch_size{32};
std::atomic<uint32_t> g_config_empty_cache_limit{ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT};
std::atomic<uint32_t> g_config_cooldown_windows{ADAPTIVE_DEFAULT_COOLDOWN_WINDOWS};
std::atomic<uint8_t> g_config_profile{static_cast<uint8_t>(AdaptiveProfileId::Balanced)};
std::atomic<uint64_t> g_arch_band_counters[3]{};
std::atomic<uint64_t> g_arch_band_cooldown_until[3]{};
std::atomic<uint8_t> g_arch_band_active[3]{
    static_cast<uint8_t>(AdaptiveStrategyId::SmallObject),
    static_cast<uint8_t>(AdaptiveStrategyId::MediumObject),
    static_cast<uint8_t>(AdaptiveStrategyId::LargeObject),
};
std::atomic<uint64_t> g_param_counter{0};
std::atomic<uint32_t> g_tuning_round{0};

struct AdaptiveWindowBase {
    uint64_t malloc_calls;
    uint64_t free_calls;
    uint64_t realloc_calls;
    uint64_t strategy_allocs[3];
    uint64_t pool_hits[3];
    uint64_t pool_misses[3];
    uint64_t architecture_switches;
    uint64_t parameter_decisions;
    uint64_t failure_count;
};

std::mutex g_window_mutex;
AdaptiveWindowBase g_last_window{};
std::atomic<uintptr_t> g_adaptive_page_table[ADAPTIVE_PAGE_TABLE_SIZE]{};

} // namespace

// --- Policy initialization ------------------------------------------------

static bool adaptive_streq(const char* a, const char* b) noexcept {
    return a && std::strcmp(a, b) == 0;
}

static bool adaptive_policy_alias(const char* env, const char* a, const char* b) noexcept {
    return adaptive_streq(env, a) || adaptive_streq(env, b);
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

static void adaptive_init_parameter_policy_once() noexcept {
    const char* env = std::getenv("MY_MALLOC_ADAPTIVE_PARAM_POLICY");
    if (!env || adaptive_streq(env, "heuristic")) {
        g_adaptive_parameter_policy = AdaptiveParameterPolicy::Heuristic;
    } else if (adaptive_streq(env, "static")) {
        g_adaptive_parameter_policy = AdaptiveParameterPolicy::Static;
    } else if (adaptive_policy_alias(env, "coordinate_bandit", "coordinate")) {
        g_adaptive_parameter_policy = AdaptiveParameterPolicy::CoordinateBandit;
    } else if (adaptive_policy_alias(env, "bayesian_offline", "bayesian")) {
        g_adaptive_parameter_policy = AdaptiveParameterPolicy::BayesianOffline;
    } else {
        g_adaptive_parameter_policy = AdaptiveParameterPolicy::Heuristic;
    }
}

static void adaptive_init_parameter_policy() noexcept {
    pthread_once(&g_adaptive_parameter_policy_once, adaptive_init_parameter_policy_once);
}

static AdaptiveProfileId parse_profile(const char* env) noexcept {
    if (adaptive_streq(env, "low_latency")) return AdaptiveProfileId::LowLatency;
    if (adaptive_streq(env, "low_rss")) return AdaptiveProfileId::LowRss;
    if (adaptive_streq(env, "large_heavy")) return AdaptiveProfileId::LargeHeavy;
    if (adaptive_streq(env, "cross_thread")) return AdaptiveProfileId::CrossThread;
    return AdaptiveProfileId::Balanced;
}

static void adaptive_init_runtime_config_once() noexcept {
    adaptive_init_parameter_policy();
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
    g_config_architecture_window.store(static_cast<uint32_t>(
        adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_ARCH_WINDOW",
                               ADAPTIVE_DEFAULT_ARCH_WINDOW,
                               64,
                               1 << 20)),
        std::memory_order_relaxed);
    g_config_parameter_window.store(static_cast<uint32_t>(
        adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_PARAM_WINDOW",
                               ADAPTIVE_DEFAULT_PARAM_WINDOW,
                               256,
                               1 << 20)),
        std::memory_order_relaxed);
    g_config_local_batch_size.store(static_cast<uint32_t>(
        adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_LOCAL_BATCH",
                               32,
                               1,
                               1024)),
        std::memory_order_relaxed);
    g_config_empty_cache_limit.store(static_cast<uint32_t>(
        adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_EMPTY_CACHE_LIMIT",
                               ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT,
                               0,
                               128)),
        std::memory_order_relaxed);
    g_config_cooldown_windows.store(static_cast<uint32_t>(
        adaptive_parse_u64_env("MY_MALLOC_ADAPTIVE_COOLDOWN_WINDOWS",
                               ADAPTIVE_DEFAULT_COOLDOWN_WINDOWS,
                               0,
                               1024)),
        std::memory_order_relaxed);
    g_config_profile.store(static_cast<uint8_t>(
        parse_profile(std::getenv("MY_MALLOC_ADAPTIVE_PROFILE"))),
        std::memory_order_relaxed);
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
    cfg.architecture_window = g_config_architecture_window.load(std::memory_order_relaxed);
    cfg.parameter_window = g_config_parameter_window.load(std::memory_order_relaxed);
    cfg.local_batch_size = g_config_local_batch_size.load(std::memory_order_relaxed);
    cfg.empty_cache_limit = g_config_empty_cache_limit.load(std::memory_order_relaxed);
    cfg.cooldown_windows = g_config_cooldown_windows.load(std::memory_order_relaxed);
    cfg.profile = static_cast<AdaptiveProfileId>(g_config_profile.load(std::memory_order_relaxed));
    return cfg;
}

static void adaptive_init_policy_once() noexcept {
    adaptive_init_runtime_config();

    const char* env = std::getenv("MY_MALLOC_ADAPTIVE_POLICY");
    if (!env || adaptive_streq(env, "heuristic")) {
        g_adaptive_policy = AdaptiveInternalPolicy::Heuristic;
    } else if (adaptive_streq(env, "fixed:small")) {
        g_adaptive_policy = AdaptiveInternalPolicy::FixedSmall;
    } else if (adaptive_streq(env, "fixed:medium")) {
        g_adaptive_policy = AdaptiveInternalPolicy::FixedMedium;
    } else if (adaptive_streq(env, "fixed:large")) {
        g_adaptive_policy = AdaptiveInternalPolicy::FixedLarge;
    } else if (adaptive_streq(env, "round_robin")) {
        g_adaptive_policy = AdaptiveInternalPolicy::RoundRobin;
    } else if (adaptive_policy_alias(env, "epsilon_greedy", "eps_greedy")) {
        g_adaptive_policy = AdaptiveInternalPolicy::EpsilonGreedy;
    } else if (adaptive_policy_alias(env, "ucb1", "ucb")) {
        g_adaptive_policy = AdaptiveInternalPolicy::Ucb1;
    } else if (adaptive_policy_alias(env, "thompson", "thompson_sampling")) {
        g_adaptive_policy = AdaptiveInternalPolicy::ThompsonSampling;
    } else {
        g_adaptive_policy = AdaptiveInternalPolicy::Heuristic;
    }
}

static void adaptive_init_policy() noexcept {
    pthread_once(&g_adaptive_policy_once, adaptive_init_policy_once);
}

// --- Strategy selection ----------------------------------------------------

static AdaptiveStrategyId heuristic_strategy(size_t size) noexcept {
    if (size <= 1024) return AdaptiveStrategyId::SmallObject;
    if (size <= 64 * 1024) return AdaptiveStrategyId::MediumObject;
    return AdaptiveStrategyId::LargeObject;
}

static size_t strategy_index(AdaptiveStrategyId id) noexcept;
static bool adaptive_page_maybe_owned(void* ptr) noexcept;
static void register_adaptive_region(void* base, size_t size) noexcept;
static void unregister_adaptive_region(void* base, size_t size) noexcept;

static size_t size_band_index(size_t size) noexcept {
    if (size <= ADAPTIVE_SMALL_MAX) return 0;
    if (size <= ADAPTIVE_MEDIUM_MAX) return 1;
    return 2;
}

static bool strategy_can_allocate(AdaptiveStrategyId strategy, size_t size) noexcept {
    switch (strategy) {
        case AdaptiveStrategyId::SmallObject:
            return size <= ADAPTIVE_SMALL_MAX;
        case AdaptiveStrategyId::MediumObject:
            return size <= ADAPTIVE_MEDIUM_MAX;
        case AdaptiveStrategyId::LargeObject:
            return true;
    }
    return false;
}

static uint64_t adaptive_rand64() noexcept {
    static std::atomic<uint64_t> state{0x9E3779B97F4A7C15ull};
    uint64_t x = state.fetch_add(0x9E3779B97F4A7C15ull, std::memory_order_relaxed);
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ull;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBull;
    x ^= x >> 31;
    return x;
}

static double adaptive_unit_random() noexcept {
    return static_cast<double>(adaptive_rand64() >> 11) * (1.0 / 9007199254740992.0);
}

static size_t candidate_strategies(size_t size, AdaptiveStrategyId out[3]) noexcept {
    size_t n = 0;
    if (size <= ADAPTIVE_SMALL_MAX) {
        out[n++] = AdaptiveStrategyId::SmallObject;
        out[n++] = AdaptiveStrategyId::MediumObject;
        out[n++] = AdaptiveStrategyId::LargeObject;
    } else if (size <= ADAPTIVE_MEDIUM_MAX) {
        out[n++] = AdaptiveStrategyId::MediumObject;
        out[n++] = AdaptiveStrategyId::LargeObject;
    } else {
        out[n++] = AdaptiveStrategyId::LargeObject;
    }
    return n;
}

static double telemetry_score(AdaptiveStrategyId strategy) noexcept {
    size_t idx = strategy_index(strategy);
    const AdaptiveStrategyStats& s = g_adaptive_stats.strategy[idx];
    double successes = static_cast<double>(s.policy_successes.load(std::memory_order_relaxed));
    double failures = static_cast<double>(s.policy_failures.load(std::memory_order_relaxed));
    double trials = successes + failures;
    double success_rate = (successes + 1.0) / (trials + 2.0);
    double latency = static_cast<double>(s.avg_alloc_latency_ns.load(std::memory_order_relaxed));
    if (latency <= 0.0) latency = 1000.0;
    double hits = static_cast<double>(s.pool_hits.load(std::memory_order_relaxed));
    double misses = static_cast<double>(s.pool_misses.load(std::memory_order_relaxed));
    double hit_rate = (hits + 1.0) / (hits + misses + 2.0);

    int64_t live = g_adaptive_stats.live_bytes.load(std::memory_order_relaxed);
    int64_t mapped = g_adaptive_stats.mapped_bytes.load(std::memory_order_relaxed);
    double pressure = 0.0;
    if (live > 0 && mapped > live) {
        pressure = static_cast<double>(mapped - live) / static_cast<double>(live);
    }
    double large_penalty = strategy == AdaptiveStrategyId::LargeObject ? 0.25 : 0.0;

    return (success_rate * 4.0) + (hit_rate * 1.5) -
           std::log1p(latency) * 0.08 - pressure * 0.05 - large_penalty;
}

static AdaptiveStrategyId best_telemetry_strategy(const AdaptiveStrategyId* candidates,
                                                  size_t count) noexcept {
    AdaptiveStrategyId best = candidates[0];
    double best_score = telemetry_score(best);
    for (size_t i = 1; i < count; ++i) {
        double score = telemetry_score(candidates[i]);
        if (score > best_score) {
            best = candidates[i];
            best_score = score;
        }
    }
    return best;
}

static AdaptiveStrategyId profile_preferred_strategy(size_t size,
                                                     AdaptiveProfileId profile) noexcept {
    switch (profile) {
        case AdaptiveProfileId::LowLatency:
            if (size <= ADAPTIVE_MEDIUM_MAX) return size <= ADAPTIVE_SMALL_MAX
                ? AdaptiveStrategyId::SmallObject
                : AdaptiveStrategyId::MediumObject;
            return AdaptiveStrategyId::LargeObject;
        case AdaptiveProfileId::LowRss:
            return heuristic_strategy(size);
        case AdaptiveProfileId::LargeHeavy:
            if (size > ADAPTIVE_SMALL_MAX) return AdaptiveStrategyId::LargeObject;
            return AdaptiveStrategyId::SmallObject;
        case AdaptiveProfileId::CrossThread:
        case AdaptiveProfileId::Balanced:
            return heuristic_strategy(size);
    }
    return heuristic_strategy(size);
}

static AdaptiveStrategyId windowed_architecture_select(size_t size,
                                                       const AdaptiveStrategyId* candidates,
                                                       size_t count,
                                                       AdaptiveStrategyId (*selector)(const AdaptiveStrategyId*, size_t)) noexcept {
    size_t band = size_band_index(size);
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    uint64_t n = g_arch_band_counters[band].fetch_add(1, std::memory_order_relaxed);
    uint64_t cooldown_until = g_arch_band_cooldown_until[band].load(std::memory_order_relaxed);
    if (n == 0 || ((n % cfg.architecture_window) == 0 && n >= cooldown_until)) {
        AdaptiveStrategyId chosen = selector(candidates, count);
        AdaptiveStrategyId profile_preferred = profile_preferred_strategy(size, cfg.profile);
        if (strategy_can_allocate(profile_preferred, size) &&
            cfg.profile != AdaptiveProfileId::Balanced) {
            chosen = profile_preferred;
        }
        uint8_t old = g_arch_band_active[band].exchange(static_cast<uint8_t>(chosen),
                                                        std::memory_order_relaxed);
        if (old != static_cast<uint8_t>(chosen)) {
            g_adaptive_stats.architecture_switches.fetch_add(1, std::memory_order_relaxed);
            uint64_t next_allowed = n + static_cast<uint64_t>(cfg.cooldown_windows) *
                static_cast<uint64_t>(cfg.architecture_window);
            g_arch_band_cooldown_until[band].store(next_allowed, std::memory_order_relaxed);
        }
        return chosen;
    }
    AdaptiveStrategyId active = static_cast<AdaptiveStrategyId>(
        g_arch_band_active[band].load(std::memory_order_relaxed));
    return strategy_can_allocate(active, size) ? active : heuristic_strategy(size);
}

static AdaptiveStrategyId epsilon_greedy_select(const AdaptiveStrategyId* candidates,
                                                size_t count) noexcept {
    constexpr double epsilon = 0.125;
    if (count > 1 && adaptive_unit_random() < epsilon) {
        return candidates[adaptive_rand64() % count];
    }
    return best_telemetry_strategy(candidates, count);
}

static AdaptiveStrategyId ucb1_select(const AdaptiveStrategyId* candidates,
                                      size_t count) noexcept {
    uint64_t total = 1;
    for (size_t i = 0; i < AdaptiveStats::NUM_STRATEGIES; ++i) {
        total += g_adaptive_stats.strategy[i].policy_trials.load(std::memory_order_relaxed);
    }

    AdaptiveStrategyId best = candidates[0];
    double best_score = -1.0e100;
    for (size_t i = 0; i < count; ++i) {
        AdaptiveStrategyId strategy = candidates[i];
        size_t idx = strategy_index(strategy);
        uint64_t trials = g_adaptive_stats.strategy[idx].policy_trials.load(std::memory_order_relaxed);
        if (trials == 0) return strategy;
        double exploit = telemetry_score(strategy);
        double explore = std::sqrt((2.0 * std::log(static_cast<double>(total))) /
                                   static_cast<double>(trials));
        double score = exploit + explore;
        if (score > best_score) {
            best = strategy;
            best_score = score;
        }
    }
    return best;
}

static AdaptiveStrategyId thompson_select(const AdaptiveStrategyId* candidates,
                                          size_t count) noexcept {
    AdaptiveStrategyId best = candidates[0];
    double best_sample = -1.0e100;
    for (size_t i = 0; i < count; ++i) {
        AdaptiveStrategyId strategy = candidates[i];
        size_t idx = strategy_index(strategy);
        const AdaptiveStrategyStats& s = g_adaptive_stats.strategy[idx];
        double alpha = static_cast<double>(s.policy_successes.load(std::memory_order_relaxed)) + 1.0;
        double beta = static_cast<double>(s.policy_failures.load(std::memory_order_relaxed)) + 1.0;
        double mean = alpha / (alpha + beta);
        double variance = (alpha * beta) /
            ((alpha + beta) * (alpha + beta) * (alpha + beta + 1.0));
        double jitter = (adaptive_unit_random() * 2.0) - 1.0;
        double sample = mean + jitter * std::sqrt(variance) + telemetry_score(strategy) * 0.05;
        if (sample > best_sample) {
            best = strategy;
            best_sample = sample;
        }
    }
    return best;
}

static AdaptiveStrategyId select_strategy(size_t size) noexcept {
    adaptive_init_policy();
    g_adaptive_stats.policy_decisions.fetch_add(1, std::memory_order_relaxed);

    switch (g_adaptive_policy) {
        case AdaptiveInternalPolicy::Heuristic:
            return heuristic_strategy(size);

        case AdaptiveInternalPolicy::FixedSmall:
            if (size <= 1024) return AdaptiveStrategyId::SmallObject;
            if (size <= 64 * 1024) return AdaptiveStrategyId::MediumObject;
            return AdaptiveStrategyId::LargeObject;

        case AdaptiveInternalPolicy::FixedMedium:
            if (size <= 64 * 1024) return AdaptiveStrategyId::MediumObject;
            return AdaptiveStrategyId::LargeObject;

        case AdaptiveInternalPolicy::FixedLarge:
            return AdaptiveStrategyId::LargeObject;

        case AdaptiveInternalPolicy::RoundRobin: {
            uint64_t n = g_adaptive_rr_counter.fetch_add(1, std::memory_order_relaxed);
            switch (n % 3) {
                case 0: return AdaptiveStrategyId::SmallObject;
                case 1: return AdaptiveStrategyId::MediumObject;
                default: return AdaptiveStrategyId::LargeObject;
            }
        }

        case AdaptiveInternalPolicy::EpsilonGreedy: {
            AdaptiveStrategyId candidates[3];
            size_t count = candidate_strategies(size, candidates);
            return windowed_architecture_select(size, candidates, count, epsilon_greedy_select);
        }

        case AdaptiveInternalPolicy::Ucb1: {
            AdaptiveStrategyId candidates[3];
            size_t count = candidate_strategies(size, candidates);
            return windowed_architecture_select(size, candidates, count, ucb1_select);
        }

        case AdaptiveInternalPolicy::ThompsonSampling: {
            AdaptiveStrategyId candidates[3];
            size_t count = candidate_strategies(size, candidates);
            return windowed_architecture_select(size, candidates, count, thompson_select);
        }
    }
    return heuristic_strategy(size);
}

// --- Strategy index helper -------------------------------------------------

static size_t strategy_index(AdaptiveStrategyId id) noexcept {
    switch (id) {
        case AdaptiveStrategyId::SmallObject:  return 0;
        case AdaptiveStrategyId::MediumObject: return 1;
        case AdaptiveStrategyId::LargeObject:  return 2;
    }
    return 0;
}

// --- Header helpers --------------------------------------------------------

static void* user_from_header(AdaptiveHeader* hdr) noexcept {
    return reinterpret_cast<char*>(hdr) + ADAPTIVE_HDR_OFFSET;
}

static bool valid_strategy_id(AdaptiveStrategyId id) noexcept {
    return id == AdaptiveStrategyId::SmallObject ||
           id == AdaptiveStrategyId::MediumObject ||
           id == AdaptiveStrategyId::LargeObject;
}

static bool header_is_plausible(AdaptiveHeader* hdr, void* user) noexcept {
    if (!hdr || hdr->magic != ADAPTIVE_MAGIC) return false;
    if (!valid_strategy_id(hdr->strategy)) return false;
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

static void record_alloc_stats(AdaptiveStrategyId strategy, size_t size,
                               size_t usable) noexcept {
    size_t idx = strategy_index(strategy);
    g_adaptive_stats.strategy[idx].alloc_count.fetch_add(1, std::memory_order_relaxed);
    g_adaptive_stats.strategy[idx].requested_bytes.fetch_add(size, std::memory_order_relaxed);
    g_adaptive_stats.strategy[idx].usable_bytes.fetch_add(usable, std::memory_order_relaxed);
    g_adaptive_stats.live_bytes.fetch_add(static_cast<int64_t>(usable), std::memory_order_relaxed);
}

static void record_free_stats(AdaptiveHeader* hdr) noexcept {
    size_t idx = strategy_index(hdr->strategy);
    g_adaptive_stats.strategy[idx].free_count.fetch_add(1, std::memory_order_relaxed);
    g_adaptive_stats.live_bytes.fetch_sub(static_cast<int64_t>(hdr->usable), std::memory_order_relaxed);
}

static void record_mapped_bytes(size_t mapped) noexcept {
    g_adaptive_stats.mapped_bytes.fetch_add(static_cast<int64_t>(mapped), std::memory_order_relaxed);
}

static void record_unmapped_bytes(size_t mapped) noexcept {
    g_adaptive_stats.mapped_bytes.fetch_sub(static_cast<int64_t>(mapped), std::memory_order_relaxed);
}

static uint64_t monotonic_time_ns() noexcept {
    using clock = std::chrono::steady_clock;
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            clock::now().time_since_epoch()).count());
}

static void record_policy_result(AdaptiveStrategyId strategy,
                                 bool success,
                                 uint64_t latency_ns) noexcept {
    size_t idx = strategy_index(strategy);
    AdaptiveStrategyStats& s = g_adaptive_stats.strategy[idx];
    s.policy_trials.fetch_add(1, std::memory_order_relaxed);
    if (success) {
        s.policy_successes.fetch_add(1, std::memory_order_relaxed);
    } else {
        s.policy_failures.fetch_add(1, std::memory_order_relaxed);
    }

    uint64_t old = s.avg_alloc_latency_ns.load(std::memory_order_relaxed);
    uint64_t next = old == 0 ? latency_ns : ((old * 7) + latency_ns) / 8;
    s.avg_alloc_latency_ns.store(next, std::memory_order_relaxed);
}

static void record_pool_hit(AdaptiveStrategyId strategy) noexcept {
    g_adaptive_stats.strategy[strategy_index(strategy)].pool_hits.fetch_add(1, std::memory_order_relaxed);
}

static void record_pool_miss(AdaptiveStrategyId strategy) noexcept {
    g_adaptive_stats.strategy[strategy_index(strategy)].pool_misses.fetch_add(1, std::memory_order_relaxed);
}

static void adaptive_update_config(size_t small_page,
                                   size_t medium_span,
                                   uint32_t local_batch,
                                   uint32_t empty_cache_limit,
                                   AdaptiveProfileId profile) noexcept {
    std::lock_guard<std::mutex> lock(g_adaptive_config_mutex);
    bool changed = false;
    small_page = adaptive_page_aligned(small_page);
    medium_span = adaptive_page_aligned(medium_span);
    if (small_page != g_config_small_page_size.load(std::memory_order_relaxed)) {
        g_config_small_page_size.store(small_page, std::memory_order_relaxed);
        changed = true;
    }
    if (medium_span != g_config_medium_span_size.load(std::memory_order_relaxed)) {
        g_config_medium_span_size.store(medium_span, std::memory_order_relaxed);
        changed = true;
    }
    if (local_batch != g_config_local_batch_size.load(std::memory_order_relaxed)) {
        g_config_local_batch_size.store(local_batch, std::memory_order_relaxed);
        changed = true;
    }
    if (empty_cache_limit != g_config_empty_cache_limit.load(std::memory_order_relaxed)) {
        g_config_empty_cache_limit.store(empty_cache_limit, std::memory_order_relaxed);
        changed = true;
    }
    if (static_cast<uint8_t>(profile) != g_config_profile.load(std::memory_order_relaxed)) {
        g_config_profile.store(static_cast<uint8_t>(profile), std::memory_order_relaxed);
        changed = true;
    }
    if (changed) {
        g_config_version.fetch_add(1, std::memory_order_relaxed);
        g_adaptive_stats.parameter_decisions.fetch_add(1, std::memory_order_relaxed);
    }
}

static AdaptiveWindowSnapshot adaptive_window_delta_locked() noexcept {
    AdaptiveWindowSnapshot delta{};
    uint64_t malloc_calls = g_adaptive_stats.malloc_calls.load(std::memory_order_relaxed);
    uint64_t free_calls = g_adaptive_stats.free_calls.load(std::memory_order_relaxed);
    uint64_t realloc_calls = g_adaptive_stats.realloc_calls.load(std::memory_order_relaxed);
    uint64_t arch_switches = g_adaptive_stats.architecture_switches.load(std::memory_order_relaxed);
    uint64_t param_decisions = g_adaptive_stats.parameter_decisions.load(std::memory_order_relaxed);
    uint64_t failures = g_adaptive_stats.failure_count.load(std::memory_order_relaxed);

    delta.alloc_calls = malloc_calls - g_last_window.malloc_calls;
    delta.free_calls = free_calls - g_last_window.free_calls;
    delta.realloc_calls = realloc_calls - g_last_window.realloc_calls;
    delta.architecture_switches = arch_switches - g_last_window.architecture_switches;
    delta.parameter_decisions = param_decisions - g_last_window.parameter_decisions;
    delta.failure_count = failures - g_last_window.failure_count;

    g_last_window.malloc_calls = malloc_calls;
    g_last_window.free_calls = free_calls;
    g_last_window.realloc_calls = realloc_calls;
    g_last_window.architecture_switches = arch_switches;
    g_last_window.parameter_decisions = param_decisions;
    g_last_window.failure_count = failures;

    for (size_t i = 0; i < AdaptiveStats::NUM_STRATEGIES; ++i) {
        uint64_t allocs = g_adaptive_stats.strategy[i].alloc_count.load(std::memory_order_relaxed);
        uint64_t hits = g_adaptive_stats.strategy[i].pool_hits.load(std::memory_order_relaxed);
        uint64_t misses = g_adaptive_stats.strategy[i].pool_misses.load(std::memory_order_relaxed);
        delta.strategy_allocs[i] = allocs - g_last_window.strategy_allocs[i];
        delta.pool_hits[i] = hits - g_last_window.pool_hits[i];
        delta.pool_misses[i] = misses - g_last_window.pool_misses[i];
        g_last_window.strategy_allocs[i] = allocs;
        g_last_window.pool_hits[i] = hits;
        g_last_window.pool_misses[i] = misses;
    }

    delta.live_bytes = g_adaptive_stats.live_bytes.load(std::memory_order_relaxed);
    delta.mapped_bytes = g_adaptive_stats.mapped_bytes.load(std::memory_order_relaxed);
    return delta;
}

static void adaptive_tune_parameters_if_needed() noexcept {
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    uint64_t n = g_param_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    if ((n % cfg.parameter_window) != 0) return;

    adaptive_init_parameter_policy();
    if (g_adaptive_parameter_policy == AdaptiveParameterPolicy::Static ||
        g_adaptive_parameter_policy == AdaptiveParameterPolicy::BayesianOffline) {
        return;
    }

    AdaptiveWindowSnapshot window{};
    {
        std::lock_guard<std::mutex> lock(g_window_mutex);
        window = adaptive_window_delta_locked();
    }
    int64_t live = window.live_bytes;
    int64_t mapped = window.mapped_bytes;
    uint64_t small_hits = window.pool_hits[strategy_index(AdaptiveStrategyId::SmallObject)];
    uint64_t small_misses = window.pool_misses[strategy_index(AdaptiveStrategyId::SmallObject)];
    uint64_t medium_hits = window.pool_hits[strategy_index(AdaptiveStrategyId::MediumObject)];
    uint64_t medium_misses = window.pool_misses[strategy_index(AdaptiveStrategyId::MediumObject)];

    size_t next_small = cfg.small_page_size;
    size_t next_medium = cfg.medium_span_size;
    uint32_t next_batch = cfg.local_batch_size;
    uint32_t next_empty_keep = cfg.empty_cache_limit;
    AdaptiveProfileId next_profile = cfg.profile;

    if (g_adaptive_parameter_policy == AdaptiveParameterPolicy::Heuristic) {
        bool high_pressure = live > 0 && mapped > live * 3;
        bool small_miss_heavy = small_misses > small_hits + 4;
        bool medium_miss_heavy = medium_misses > medium_hits + 2;
        if (high_pressure) {
            if (next_small > 32 * 1024) next_small /= 2;
            if (next_medium > 128 * 1024) next_medium /= 2;
            if (next_batch > 8) next_batch /= 2;
            next_empty_keep = 0;
            next_profile = AdaptiveProfileId::LowRss;
        } else {
            if (small_miss_heavy && next_small < 256 * 1024) next_small *= 2;
            if (medium_miss_heavy && next_medium < 1024 * 1024) next_medium *= 2;
            if ((small_miss_heavy || medium_miss_heavy) && next_batch < 256) next_batch *= 2;
            if (small_miss_heavy || medium_miss_heavy) {
                next_empty_keep = 4;
                next_profile = AdaptiveProfileId::LowLatency;
            } else {
                next_profile = AdaptiveProfileId::Balanced;
            }
        }
    } else if (g_adaptive_parameter_policy == AdaptiveParameterPolicy::CoordinateBandit) {
        static constexpr size_t small_candidates[] = {32 * 1024, 64 * 1024, 128 * 1024, 256 * 1024};
        static constexpr size_t medium_candidates[] = {128 * 1024, 256 * 1024, 512 * 1024, 1024 * 1024};
        static constexpr uint32_t batch_candidates[] = {8, 16, 32, 64, 128};
        uint32_t round = g_tuning_round.fetch_add(1, std::memory_order_relaxed);
        switch (round % 3) {
            case 0:
                next_small = small_candidates[(round / 3) % 4];
                break;
            case 1:
                next_medium = medium_candidates[(round / 3) % 4];
                break;
            default:
                next_batch = batch_candidates[(round / 3) % 5];
                break;
        }
    }

    adaptive_update_config(next_small, next_medium, next_batch, next_empty_keep, next_profile);
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

static AdaptivePage* create_pool_page(AdaptiveStrategyId strategy,
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
    page->strategy = strategy;
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
        hdr->strategy = strategy;
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
        page->free_list = hdr;
    }

    record_mapped_bytes(mapped_size);
    return page;
}

static AdaptivePage** pool_head_for(AdaptiveStrategyId strategy, size_t class_index) noexcept {
    if (strategy == AdaptiveStrategyId::SmallObject) {
        return class_index < ADAPTIVE_SMALL_CLASS_COUNT ? &g_small_pages[class_index] : nullptr;
    }
    if (strategy == AdaptiveStrategyId::MediumObject) {
        return class_index < ADAPTIVE_MEDIUM_CLASS_COUNT ? &g_medium_pages[class_index] : nullptr;
    }
    return nullptr;
}

static std::mutex* pool_mutex_for(AdaptiveStrategyId strategy, size_t class_index) noexcept {
    if (strategy == AdaptiveStrategyId::SmallObject) {
        return class_index < ADAPTIVE_SMALL_CLASS_COUNT ? &g_small_class_mutexes[class_index] : nullptr;
    }
    if (strategy == AdaptiveStrategyId::MediumObject) {
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
    AdaptiveStrategyId strategy = target->strategy;
    record_unmapped_bytes(mapped);
    g_adaptive_stats.release_unmapped_bytes.fetch_add(mapped, std::memory_order_relaxed);
    if (strategy == AdaptiveStrategyId::SmallObject) {
        g_adaptive_stats.released_pages.fetch_add(1, std::memory_order_relaxed);
    } else if (strategy == AdaptiveStrategyId::MediumObject) {
        g_adaptive_stats.released_spans.fetch_add(1, std::memory_order_relaxed);
    }
    unregister_adaptive_region(target, mapped);
    munmap(target, mapped);
}

static void release_empty_pages_if_needed_locked(AdaptiveStrategyId strategy,
                                                 AdaptivePage** head,
                                                 uint32_t keep_limit) noexcept {
    if (!head) return;
    uint32_t empty = count_empty_pages_locked(*head);
    if (strategy == AdaptiveStrategyId::SmallObject) {
        g_adaptive_stats.empty_pages.store(empty, std::memory_order_relaxed);
    } else if (strategy == AdaptiveStrategyId::MediumObject) {
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

static void* adaptive_pool_alloc(size_t size, AdaptiveStrategyId strategy) noexcept {
    size_t class_index = 0;
    size_t usable = 0;
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    size_t mapped_size = 0;
    if (strategy == AdaptiveStrategyId::SmallObject) {
        if (!small_class(size, class_index, usable)) return nullptr;
        mapped_size = cfg.small_page_size;
    } else if (strategy == AdaptiveStrategyId::MediumObject) {
        if (!medium_class(size, class_index, usable)) return nullptr;
        mapped_size = cfg.medium_span_size;
    } else {
        return nullptr;
    }

    AdaptiveHeader* hdr = nullptr;
    {
        AdaptivePage** head = pool_head_for(strategy, class_index);
        std::mutex* mutex = pool_mutex_for(strategy, class_index);
        if (!head || !mutex) return nullptr;
        std::lock_guard<std::mutex> lock(*mutex);

        AdaptivePage* page = *head;
        while (page && !page->free_list) {
            page = page->next;
        }
        if (!page) {
            record_pool_miss(strategy);
            page = create_pool_page(strategy, class_index, usable, mapped_size, cfg.version);
            if (!page) return nullptr;
            page->next = *head;
            *head = page;
        } else {
            record_pool_hit(strategy);
        }

        hdr = page->free_list;
        page->free_list = hdr->registry_next;
        page->live_count++;

        hdr->magic = ADAPTIVE_MAGIC;
        hdr->strategy = strategy;
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
    }

    registry_insert(hdr);
    record_alloc_stats(strategy, size, usable);
    return user_from_header(hdr);
}

static void adaptive_pool_free(AdaptiveHeader* hdr) noexcept {
    registry_remove(hdr);
    record_free_stats(hdr);

    AdaptivePage* page = hdr->owner_page;
    if (!page) return;
    AdaptiveStrategyId strategy = page->strategy;
    size_t class_index = page->class_index;
    AdaptivePage** head = pool_head_for(strategy, class_index);
    std::mutex* mutex = pool_mutex_for(strategy, class_index);
    if (!head || !mutex) return;
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    std::lock_guard<std::mutex> lock(*mutex);
    hdr->magic = 0;
    hdr->requested = 0;
    hdr->registry_prev = nullptr;
    hdr->registry_next = page->free_list;
    page->free_list = hdr;
    if (page->live_count > 0) {
        page->live_count--;
    }
    release_empty_pages_if_needed_locked(strategy, head, cfg.empty_cache_limit);
}

static void* adaptive_mmap_alloc(size_t size, AdaptiveStrategyId strategy) noexcept {
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
    hdr->strategy    = strategy;
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

    record_mapped_bytes(mapped);
    registry_insert(hdr);
    record_alloc_stats(strategy, size, usable);
    return user_from_header(hdr);
}

static void adaptive_mmap_free(AdaptiveHeader* hdr) noexcept {
    registry_remove(hdr);
    record_free_stats(hdr);
    void* base = hdr->region_base;
    size_t mapped = hdr->mapped_size;
    record_unmapped_bytes(mapped);
    unregister_adaptive_region(base, mapped);
    munmap(base, mapped);
}

static void* allocate_with_strategy(size_t size, AdaptiveStrategyId strategy) noexcept {
    if (strategy == AdaptiveStrategyId::SmallObject ||
        strategy == AdaptiveStrategyId::MediumObject) {
        return adaptive_pool_alloc(size, strategy);
    }
    return adaptive_mmap_alloc(size, AdaptiveStrategyId::LargeObject);
}

// --- Public API ------------------------------------------------------------

void* adaptive_malloc(size_t size) noexcept {
    g_adaptive_stats.malloc_calls.fetch_add(1, std::memory_order_relaxed);

    uint64_t start_ns = monotonic_time_ns();
    AdaptiveStrategyId strategy = select_strategy(size);
    AdaptiveStrategyId actual_strategy = strategy;
    void* ptr = allocate_with_strategy(size, strategy);
    if (!ptr) {
        record_policy_result(strategy, false, monotonic_time_ns() - start_ns);
        AdaptiveStrategyId fallback = heuristic_strategy(size);
        actual_strategy = fallback;
        if (fallback == AdaptiveStrategyId::SmallObject ||
            fallback == AdaptiveStrategyId::MediumObject) {
            ptr = adaptive_pool_alloc(size, fallback);
        } else {
            ptr = adaptive_mmap_alloc(size, AdaptiveStrategyId::LargeObject);
        }
    }
    if (!ptr) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
    }
    record_policy_result(actual_strategy, ptr != nullptr, monotonic_time_ns() - start_ns);
    adaptive_tune_parameters_if_needed();
    return ptr;
}

void adaptive_free(void* ptr) noexcept {
    if (!ptr) return;
    g_adaptive_stats.free_calls.fetch_add(1, std::memory_order_relaxed);

    AdaptiveHeader* hdr = registry_find(ptr);
    if (!hdr) {
        // Not adaptive-owned. Do NOT pass to ptmalloc/slab/family free.
        return;
    }

    if (hdr->flags & ADAPTIVE_FLAG_POOLED) {
        adaptive_pool_free(hdr);
    } else {
        adaptive_mmap_free(hdr);
    }
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
        return nullptr;
    }

    size_t old_usable = hdr->usable;
    if (size <= old_usable) {
        return ptr;
    }

    void* new_ptr = adaptive_malloc(size);
    if (!new_ptr) return nullptr;

    size_t copy = old_usable < size ? old_usable : size;
    std::memcpy(new_ptr, ptr, copy);

    adaptive_free(ptr);
    return new_ptr;
}

size_t adaptive_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;
    AdaptiveHeader* hdr = registry_find(ptr);
    if (!hdr) return 0;
    return hdr->usable;
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
    uint64_t start_ns = monotonic_time_ns();
    AdaptiveStrategyId strategy = select_strategy(size);

    size_t usable = 0;
    if (!round_usable(size, usable)) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
        record_policy_result(strategy, false, monotonic_time_ns() - start_ns);
        return nullptr;
    }
    // Over-allocate: header + usable + alignment for shifting
    if (usable > static_cast<size_t>(-1) - ADAPTIVE_HDR_OFFSET ||
        alignment > static_cast<size_t>(-1) - ADAPTIVE_HDR_OFFSET - usable) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
        record_policy_result(strategy, false, monotonic_time_ns() - start_ns);
        return nullptr;
    }
    size_t total = ADAPTIVE_HDR_OFFSET + usable + alignment;
    constexpr size_t page_size = 4096;
    if (total > static_cast<size_t>(-1) - (page_size - 1)) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
        record_policy_result(strategy, false, monotonic_time_ns() - start_ns);
        return nullptr;
    }
    size_t mapped = (total + page_size - 1) & ~(page_size - 1);

    void* region = mmap(nullptr, mapped, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) {
        g_adaptive_stats.failure_count.fetch_add(1, std::memory_order_relaxed);
        record_policy_result(strategy, false, monotonic_time_ns() - start_ns);
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
    hdr->strategy    = strategy;
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

    record_mapped_bytes(mapped);
    registry_insert(hdr);
    record_alloc_stats(strategy, size, usable);
    record_policy_result(strategy, true, monotonic_time_ns() - start_ns);
    adaptive_tune_parameters_if_needed();
    return reinterpret_cast<void*>(aligned_user);
}

bool adaptive_owns(void* ptr) noexcept {
    if (!ptr) return false;
    return registry_find(ptr) != nullptr;
}

// --- Stats -----------------------------------------------------------------

AdaptiveStatsSnapshot adaptive_stats_snapshot() noexcept {
    AdaptiveStatsSnapshot s;
    s.malloc_calls     = g_adaptive_stats.malloc_calls.load(std::memory_order_relaxed);
    s.free_calls       = g_adaptive_stats.free_calls.load(std::memory_order_relaxed);
    s.realloc_calls    = g_adaptive_stats.realloc_calls.load(std::memory_order_relaxed);
    s.failure_count    = g_adaptive_stats.failure_count.load(std::memory_order_relaxed);
    s.policy_decisions = g_adaptive_stats.policy_decisions.load(std::memory_order_relaxed);
    s.architecture_switches = g_adaptive_stats.architecture_switches.load(std::memory_order_relaxed);
    s.parameter_decisions = g_adaptive_stats.parameter_decisions.load(std::memory_order_relaxed);
    s.empty_pages = g_adaptive_stats.empty_pages.load(std::memory_order_relaxed);
    s.empty_spans = g_adaptive_stats.empty_spans.load(std::memory_order_relaxed);
    s.released_pages = g_adaptive_stats.released_pages.load(std::memory_order_relaxed);
    s.released_spans = g_adaptive_stats.released_spans.load(std::memory_order_relaxed);
    s.release_unmapped_bytes = g_adaptive_stats.release_unmapped_bytes.load(std::memory_order_relaxed);

    for (size_t i = 0; i < AdaptiveStats::NUM_STRATEGIES; ++i) {
        s.strategy[i].alloc_count     = g_adaptive_stats.strategy[i].alloc_count.load(std::memory_order_relaxed);
        s.strategy[i].free_count      = g_adaptive_stats.strategy[i].free_count.load(std::memory_order_relaxed);
        s.strategy[i].requested_bytes = g_adaptive_stats.strategy[i].requested_bytes.load(std::memory_order_relaxed);
        s.strategy[i].usable_bytes    = g_adaptive_stats.strategy[i].usable_bytes.load(std::memory_order_relaxed);
        s.strategy[i].policy_trials   = g_adaptive_stats.strategy[i].policy_trials.load(std::memory_order_relaxed);
        s.strategy[i].policy_successes = g_adaptive_stats.strategy[i].policy_successes.load(std::memory_order_relaxed);
        s.strategy[i].policy_failures = g_adaptive_stats.strategy[i].policy_failures.load(std::memory_order_relaxed);
        s.strategy[i].avg_alloc_latency_ns = g_adaptive_stats.strategy[i].avg_alloc_latency_ns.load(std::memory_order_relaxed);
        s.strategy[i].pool_hits       = g_adaptive_stats.strategy[i].pool_hits.load(std::memory_order_relaxed);
        s.strategy[i].pool_misses     = g_adaptive_stats.strategy[i].pool_misses.load(std::memory_order_relaxed);
    }

    s.live_bytes   = g_adaptive_stats.live_bytes.load(std::memory_order_relaxed);
    s.mapped_bytes = g_adaptive_stats.mapped_bytes.load(std::memory_order_relaxed);
    s.mapped_live_ratio = s.live_bytes > 0
        ? static_cast<double>(s.mapped_bytes) / static_cast<double>(s.live_bytes)
        : 0.0;
    return s;
}

void adaptive_stats_reset() noexcept {
    g_adaptive_stats.malloc_calls.store(0, std::memory_order_relaxed);
    g_adaptive_stats.free_calls.store(0, std::memory_order_relaxed);
    g_adaptive_stats.realloc_calls.store(0, std::memory_order_relaxed);
    g_adaptive_stats.failure_count.store(0, std::memory_order_relaxed);
    g_adaptive_stats.policy_decisions.store(0, std::memory_order_relaxed);
    g_adaptive_stats.architecture_switches.store(0, std::memory_order_relaxed);
    g_adaptive_stats.parameter_decisions.store(0, std::memory_order_relaxed);
    g_adaptive_stats.empty_pages.store(0, std::memory_order_relaxed);
    g_adaptive_stats.empty_spans.store(0, std::memory_order_relaxed);
    g_adaptive_stats.released_pages.store(0, std::memory_order_relaxed);
    g_adaptive_stats.released_spans.store(0, std::memory_order_relaxed);
    g_adaptive_stats.release_unmapped_bytes.store(0, std::memory_order_relaxed);
    for (size_t i = 0; i < AdaptiveStats::NUM_STRATEGIES; ++i) {
        g_adaptive_stats.strategy[i].alloc_count.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].free_count.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].requested_bytes.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].usable_bytes.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].policy_trials.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].policy_successes.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].policy_failures.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].avg_alloc_latency_ns.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].pool_hits.store(0, std::memory_order_relaxed);
        g_adaptive_stats.strategy[i].pool_misses.store(0, std::memory_order_relaxed);
    }
    g_adaptive_stats.live_bytes.store(0, std::memory_order_relaxed);
    g_adaptive_stats.mapped_bytes.store(0, std::memory_order_relaxed);
    g_param_counter.store(0, std::memory_order_relaxed);
    for (size_t i = 0; i < 3; ++i) {
        g_arch_band_counters[i].store(0, std::memory_order_relaxed);
        g_arch_band_cooldown_until[i].store(0, std::memory_order_relaxed);
    }
    {
        std::lock_guard<std::mutex> lock(g_window_mutex);
        g_last_window = AdaptiveWindowBase{};
    }
}

AdaptiveInternalPolicy adaptive_current_policy() noexcept {
    adaptive_init_policy();
    return g_adaptive_policy;
}

const char* adaptive_policy_name(AdaptiveInternalPolicy p) noexcept {
    switch (p) {
        case AdaptiveInternalPolicy::Heuristic:   return "heuristic";
        case AdaptiveInternalPolicy::FixedSmall:   return "fixed:small";
        case AdaptiveInternalPolicy::FixedMedium:  return "fixed:medium";
        case AdaptiveInternalPolicy::FixedLarge:   return "fixed:large";
        case AdaptiveInternalPolicy::RoundRobin:   return "round_robin";
        case AdaptiveInternalPolicy::EpsilonGreedy: return "epsilon_greedy";
        case AdaptiveInternalPolicy::Ucb1: return "ucb1";
        case AdaptiveInternalPolicy::ThompsonSampling: return "thompson_sampling";
    }
    return "unknown";
}

AdaptiveParameterPolicy adaptive_current_parameter_policy() noexcept {
    adaptive_init_parameter_policy();
    return g_adaptive_parameter_policy;
}

const char* adaptive_parameter_policy_name(AdaptiveParameterPolicy p) noexcept {
    switch (p) {
        case AdaptiveParameterPolicy::Static:
            return "static";
        case AdaptiveParameterPolicy::Heuristic:
            return "heuristic";
        case AdaptiveParameterPolicy::CoordinateBandit:
            return "coordinate_bandit";
        case AdaptiveParameterPolicy::BayesianOffline:
            return "bayesian_offline";
    }
    return "unknown";
}

AdaptiveConfigSnapshot adaptive_config_snapshot() noexcept {
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    AdaptiveConfigSnapshot s{};
    s.version = cfg.version;
    s.parameter_policy = adaptive_current_parameter_policy();
    s.small_page_size = cfg.small_page_size;
    s.medium_span_size = cfg.medium_span_size;
    s.architecture_window = cfg.architecture_window;
    s.parameter_window = cfg.parameter_window;
    s.local_batch_size = cfg.local_batch_size;
    s.empty_cache_limit = cfg.empty_cache_limit;
    s.cooldown_windows = cfg.cooldown_windows;
    s.profile = cfg.profile;
    return s;
}

AdaptiveWindowSnapshot adaptive_window_snapshot() noexcept {
    std::lock_guard<std::mutex> lock(g_window_mutex);
    return adaptive_window_delta_locked();
}

const char* adaptive_profile_name(AdaptiveProfileId p) noexcept {
    switch (p) {
        case AdaptiveProfileId::Balanced:
            return "balanced";
        case AdaptiveProfileId::LowLatency:
            return "low_latency";
        case AdaptiveProfileId::LowRss:
            return "low_rss";
        case AdaptiveProfileId::LargeHeavy:
            return "large_heavy";
        case AdaptiveProfileId::CrossThread:
            return "cross_thread";
    }
    return "unknown";
}

} // namespace my_ptmalloc
