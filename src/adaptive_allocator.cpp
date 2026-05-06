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
#include <sys/mman.h>

namespace my_ptmalloc {

// --- Global state ----------------------------------------------------------

static AdaptiveStats g_adaptive_stats;
static bool g_adaptive_policy_initialized = false;
static AdaptiveInternalPolicy g_adaptive_policy = AdaptiveInternalPolicy::Heuristic;
static bool g_adaptive_parameter_policy_initialized = false;
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

struct AdaptiveRuntimeConfig {
    uint32_t version;
    size_t small_page_size;
    size_t medium_span_size;
    uint32_t architecture_window;
    uint32_t parameter_window;
    uint32_t local_batch_size;
};

std::mutex g_adaptive_pool_mutex;
std::mutex g_adaptive_config_mutex;
AdaptivePage* g_small_pages[ADAPTIVE_SMALL_CLASS_COUNT]{};
AdaptivePage* g_medium_pages[ADAPTIVE_MEDIUM_CLASS_COUNT]{};
std::atomic<uint32_t> g_config_version{1};
std::atomic<size_t> g_config_small_page_size{ADAPTIVE_SMALL_PAGE_SIZE};
std::atomic<size_t> g_config_medium_span_size{ADAPTIVE_MEDIUM_SPAN_SIZE};
std::atomic<uint32_t> g_config_architecture_window{ADAPTIVE_DEFAULT_ARCH_WINDOW};
std::atomic<uint32_t> g_config_parameter_window{ADAPTIVE_DEFAULT_PARAM_WINDOW};
std::atomic<uint32_t> g_config_local_batch_size{32};
std::atomic<uint64_t> g_arch_band_counters[3]{};
std::atomic<uint8_t> g_arch_band_active[3]{
    static_cast<uint8_t>(AdaptiveStrategyId::SmallObject),
    static_cast<uint8_t>(AdaptiveStrategyId::MediumObject),
    static_cast<uint8_t>(AdaptiveStrategyId::LargeObject),
};
std::atomic<uint64_t> g_param_counter{0};
std::atomic<uint32_t> g_tuning_round{0};

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

static void adaptive_init_parameter_policy() noexcept {
    if (g_adaptive_parameter_policy_initialized) return;
    g_adaptive_parameter_policy_initialized = true;

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

static void adaptive_init_runtime_config() noexcept {
    adaptive_init_parameter_policy();

    std::lock_guard<std::mutex> lock(g_adaptive_config_mutex);
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

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
    return cfg;
}

static void adaptive_init_policy() noexcept {
    if (g_adaptive_policy_initialized) return;
    g_adaptive_policy_initialized = true;
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

// --- Strategy selection ----------------------------------------------------

static AdaptiveStrategyId heuristic_strategy(size_t size) noexcept {
    if (size <= 1024) return AdaptiveStrategyId::SmallObject;
    if (size <= 64 * 1024) return AdaptiveStrategyId::MediumObject;
    return AdaptiveStrategyId::LargeObject;
}

static size_t strategy_index(AdaptiveStrategyId id) noexcept;

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
            size_t band = size_band_index(size);
            AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
            uint64_t n = g_arch_band_counters[band].fetch_add(1, std::memory_order_relaxed);
            if (n == 0 || (n % cfg.architecture_window) == 0) {
                AdaptiveStrategyId chosen = epsilon_greedy_select(candidates, count);
                uint8_t old = g_arch_band_active[band].exchange(static_cast<uint8_t>(chosen),
                                                                std::memory_order_relaxed);
                if (old != static_cast<uint8_t>(chosen)) {
                    g_adaptive_stats.architecture_switches.fetch_add(1, std::memory_order_relaxed);
                }
                return chosen;
            }
            AdaptiveStrategyId active = static_cast<AdaptiveStrategyId>(
                g_arch_band_active[band].load(std::memory_order_relaxed));
            return strategy_can_allocate(active, size) ? active : heuristic_strategy(size);
        }

        case AdaptiveInternalPolicy::Ucb1: {
            AdaptiveStrategyId candidates[3];
            size_t count = candidate_strategies(size, candidates);
            size_t band = size_band_index(size);
            AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
            uint64_t n = g_arch_band_counters[band].fetch_add(1, std::memory_order_relaxed);
            if (n == 0 || (n % cfg.architecture_window) == 0) {
                AdaptiveStrategyId chosen = ucb1_select(candidates, count);
                uint8_t old = g_arch_band_active[band].exchange(static_cast<uint8_t>(chosen),
                                                                std::memory_order_relaxed);
                if (old != static_cast<uint8_t>(chosen)) {
                    g_adaptive_stats.architecture_switches.fetch_add(1, std::memory_order_relaxed);
                }
                return chosen;
            }
            AdaptiveStrategyId active = static_cast<AdaptiveStrategyId>(
                g_arch_band_active[band].load(std::memory_order_relaxed));
            return strategy_can_allocate(active, size) ? active : heuristic_strategy(size);
        }

        case AdaptiveInternalPolicy::ThompsonSampling: {
            AdaptiveStrategyId candidates[3];
            size_t count = candidate_strategies(size, candidates);
            size_t band = size_band_index(size);
            AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
            uint64_t n = g_arch_band_counters[band].fetch_add(1, std::memory_order_relaxed);
            if (n == 0 || (n % cfg.architecture_window) == 0) {
                AdaptiveStrategyId chosen = thompson_select(candidates, count);
                uint8_t old = g_arch_band_active[band].exchange(static_cast<uint8_t>(chosen),
                                                                std::memory_order_relaxed);
                if (old != static_cast<uint8_t>(chosen)) {
                    g_adaptive_stats.architecture_switches.fetch_add(1, std::memory_order_relaxed);
                }
                return chosen;
            }
            AdaptiveStrategyId active = static_cast<AdaptiveStrategyId>(
                g_arch_band_active[band].load(std::memory_order_relaxed));
            return strategy_can_allocate(active, size) ? active : heuristic_strategy(size);
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
                                   uint32_t local_batch) noexcept {
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
    if (changed) {
        g_config_version.fetch_add(1, std::memory_order_relaxed);
        g_adaptive_stats.parameter_decisions.fetch_add(1, std::memory_order_relaxed);
    }
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

    int64_t live = g_adaptive_stats.live_bytes.load(std::memory_order_relaxed);
    int64_t mapped = g_adaptive_stats.mapped_bytes.load(std::memory_order_relaxed);
    uint64_t small_hits = g_adaptive_stats.strategy[strategy_index(AdaptiveStrategyId::SmallObject)]
        .pool_hits.load(std::memory_order_relaxed);
    uint64_t small_misses = g_adaptive_stats.strategy[strategy_index(AdaptiveStrategyId::SmallObject)]
        .pool_misses.load(std::memory_order_relaxed);
    uint64_t medium_hits = g_adaptive_stats.strategy[strategy_index(AdaptiveStrategyId::MediumObject)]
        .pool_hits.load(std::memory_order_relaxed);
    uint64_t medium_misses = g_adaptive_stats.strategy[strategy_index(AdaptiveStrategyId::MediumObject)]
        .pool_misses.load(std::memory_order_relaxed);

    size_t next_small = cfg.small_page_size;
    size_t next_medium = cfg.medium_span_size;
    uint32_t next_batch = cfg.local_batch_size;

    if (g_adaptive_parameter_policy == AdaptiveParameterPolicy::Heuristic) {
        bool high_pressure = live > 0 && mapped > live * 3;
        bool small_miss_heavy = small_misses > small_hits + 4;
        bool medium_miss_heavy = medium_misses > medium_hits + 2;
        if (high_pressure) {
            if (next_small > 32 * 1024) next_small /= 2;
            if (next_medium > 128 * 1024) next_medium /= 2;
            if (next_batch > 8) next_batch /= 2;
        } else {
            if (small_miss_heavy && next_small < 256 * 1024) next_small *= 2;
            if (medium_miss_heavy && next_medium < 1024 * 1024) next_medium *= 2;
            if ((small_miss_heavy || medium_miss_heavy) && next_batch < 256) next_batch *= 2;
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

    adaptive_update_config(next_small, next_medium, next_batch);
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
        std::lock_guard<std::mutex> lock(g_adaptive_pool_mutex);
        AdaptivePage** head = pool_head_for(strategy, class_index);
        if (!head) return nullptr;

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

    std::lock_guard<std::mutex> lock(g_adaptive_pool_mutex);
    AdaptivePage* page = hdr->owner_page;
    if (!page) return;
    hdr->magic = 0;
    hdr->requested = 0;
    hdr->registry_prev = nullptr;
    hdr->registry_next = page->free_list;
    page->free_list = hdr;
    if (page->live_count > 0) {
        page->live_count--;
    }
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
    return s;
}

} // namespace my_ptmalloc
