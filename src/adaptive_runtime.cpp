// Adaptive runtime config and soft-switch state.

#include "my_ptmalloc/adaptive_runtime.h"
#include "my_ptmalloc/adaptive_mode.h"
#include "my_ptmalloc/adaptive_telemetry.h"

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <pthread.h>

namespace my_ptmalloc {

namespace {

constexpr size_t ADAPTIVE_SMALL_PAGE_SIZE = 64 * 1024;
constexpr size_t ADAPTIVE_MEDIUM_SPAN_SIZE = 256 * 1024;
constexpr uint32_t ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT = 2;
constexpr uint32_t ADAPTIVE_DEFAULT_MODE_WINDOW = 4096;
constexpr uint32_t ADAPTIVE_DEFAULT_MODE_COOLDOWN = 2;

pthread_once_t g_runtime_once = PTHREAD_ONCE_INIT;
std::mutex g_runtime_mutex;
std::atomic<uint32_t> g_config_version{1};
std::atomic<size_t> g_small_page_size{ADAPTIVE_SMALL_PAGE_SIZE};
std::atomic<size_t> g_medium_span_size{ADAPTIVE_MEDIUM_SPAN_SIZE};
std::atomic<uint32_t> g_empty_cache_limit{ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT};
std::atomic<uint8_t> g_config_mode{static_cast<uint8_t>(AdaptiveModeId::Balanced)};
std::atomic<uint8_t> g_selector_kind{static_cast<uint8_t>(AdaptiveModeSelectorKind::Rule)};
std::atomic<uint32_t> g_mode_window{ADAPTIVE_DEFAULT_MODE_WINDOW};
std::atomic<uint32_t> g_mode_cooldown{ADAPTIVE_DEFAULT_MODE_COOLDOWN};
std::atomic<uint8_t> g_active_mode{static_cast<uint8_t>(AdaptiveModeId::Balanced)};
std::atomic<uint8_t> g_previous_mode{static_cast<uint8_t>(AdaptiveModeId::Balanced)};
std::atomic<uint64_t> g_selector_counter{0};
std::atomic<uint64_t> g_cooldown_until{0};
std::atomic<uint8_t> g_mode_states[ADAPTIVE_MODE_COUNT]{};
std::atomic<uint64_t> g_alloc_epoch{0};

static bool streq(const char* a, const char* b) noexcept {
    return a && std::strcmp(a, b) == 0;
}

static bool alias(const char* env, const char* a, const char* b) noexcept {
    return streq(env, a) || streq(env, b);
}

static uint64_t parse_u64_env(const char* name, uint64_t fallback,
                              uint64_t min_value, uint64_t max_value) noexcept {
    const char* env = std::getenv(name);
    if (!env || !*env) return fallback;
    char* end = nullptr;
    uint64_t value = std::strtoull(env, &end, 10);
    if (end == env) return fallback;
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static size_t page_aligned(size_t value) noexcept {
    constexpr size_t page = 4096;
    if (value < page) value = page;
    return (value + page - 1) & ~(page - 1);
}

static bool env_present(const char* name) noexcept {
    const char* env = std::getenv(name);
    return env && *env;
}

static AdaptiveModeId parse_mode(const char* env, bool* is_auto = nullptr) noexcept {
    if (is_auto) *is_auto = false;
    if (!env || !*env || streq(env, "auto")) {
        if (is_auto) *is_auto = true;
        return AdaptiveModeId::Balanced;
    }
    if (streq(env, "balanced")) return AdaptiveModeId::Balanced;
    if (alias(env, "throughput_cache", "throughput")) return AdaptiveModeId::ThroughputCache;
    if (alias(env, "deterministic_latency", "latency")) return AdaptiveModeId::DeterministicLatency;
    if (alias(env, "compact_rss", "rss")) return AdaptiveModeId::CompactRSS;
    if (alias(env, "fragmentation_stable", "fragmentation")) return AdaptiveModeId::FragmentationStable;
    if (alias(env, "cross_thread", "cross_thread_message")) return AdaptiveModeId::CrossThreadMessage;
    if (alias(env, "large_object", "large_object_streaming")) return AdaptiveModeId::LargeObjectStreaming;
    if (alias(env, "hardened_debug", "debug")) return AdaptiveModeId::HardenedDebug;
    return AdaptiveModeId::Balanced;
}

static AdaptiveModeSelectorKind parse_selector(const char* env) noexcept {
    if (streq(env, "fixed")) return AdaptiveModeSelectorKind::Fixed;
    if (streq(env, "manual")) return AdaptiveModeSelectorKind::Manual;
    return AdaptiveModeSelectorKind::Rule;
}

static void init_once() noexcept {
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    g_small_page_size.store(page_aligned(static_cast<size_t>(
        parse_u64_env("MY_MALLOC_ADAPTIVE_SMALL_PAGE_SIZE", ADAPTIVE_SMALL_PAGE_SIZE, 16 * 1024, 1024 * 1024))),
        std::memory_order_relaxed);
    g_medium_span_size.store(page_aligned(static_cast<size_t>(
        parse_u64_env("MY_MALLOC_ADAPTIVE_MEDIUM_SPAN_SIZE", ADAPTIVE_MEDIUM_SPAN_SIZE, 64 * 1024, 4 * 1024 * 1024))),
        std::memory_order_relaxed);
    g_empty_cache_limit.store(static_cast<uint32_t>(
        parse_u64_env("MY_MALLOC_ADAPTIVE_EMPTY_CACHE_LIMIT", ADAPTIVE_DEFAULT_EMPTY_CACHE_LIMIT, 0, 128)),
        std::memory_order_relaxed);
    g_mode_window.store(static_cast<uint32_t>(
        parse_u64_env("MY_MALLOC_ADAPTIVE_MODE_WINDOW", ADAPTIVE_DEFAULT_MODE_WINDOW, 64, 1 << 20)),
        std::memory_order_relaxed);
    g_mode_cooldown.store(static_cast<uint32_t>(
        parse_u64_env("MY_MALLOC_ADAPTIVE_MODE_COOLDOWN", ADAPTIVE_DEFAULT_MODE_COOLDOWN, 0, 1024)),
        std::memory_order_relaxed);

    bool mode_auto = false;
    bool mode_env = env_present("MY_MALLOC_ADAPTIVE_MODE");
    AdaptiveModeId configured = parse_mode(std::getenv("MY_MALLOC_ADAPTIVE_MODE"), &mode_auto);
    const char* debug = std::getenv("MY_MALLOC_ADAPTIVE_DEBUG_MODE");
    if (streq(debug, "1") || streq(debug, "true")) {
        configured = AdaptiveModeId::HardenedDebug;
        mode_auto = false;
    }
    AdaptiveModeSelectorKind selector = parse_selector(std::getenv("MY_MALLOC_ADAPTIVE_MODE_SELECTOR"));
    if (mode_auto && selector == AdaptiveModeSelectorKind::Fixed) {
        selector = AdaptiveModeSelectorKind::Rule;
    } else if (!mode_auto && mode_env && selector == AdaptiveModeSelectorKind::Rule) {
        selector = AdaptiveModeSelectorKind::Fixed;
    }

    g_config_mode.store(static_cast<uint8_t>(configured), std::memory_order_relaxed);
    g_active_mode.store(static_cast<uint8_t>(configured), std::memory_order_relaxed);
    g_previous_mode.store(static_cast<uint8_t>(configured), std::memory_order_relaxed);
    g_selector_kind.store(static_cast<uint8_t>(selector), std::memory_order_relaxed);
    for (size_t i = 0; i < ADAPTIVE_MODE_COUNT; ++i) {
        g_mode_states[i].store(static_cast<uint8_t>(AdaptiveModeState::Inactive), std::memory_order_relaxed);
    }
    g_mode_states[adaptive_mode_index(configured)].store(static_cast<uint8_t>(AdaptiveModeState::Active),
                                                         std::memory_order_relaxed);
}

} // namespace

void adaptive_runtime_init() noexcept {
    pthread_once(&g_runtime_once, init_once);
}

AdaptiveRuntimeConfig adaptive_runtime_config() noexcept {
    adaptive_runtime_init();
    return AdaptiveRuntimeConfig{
        g_config_version.load(std::memory_order_relaxed),
        g_small_page_size.load(std::memory_order_relaxed),
        g_medium_span_size.load(std::memory_order_relaxed),
        g_empty_cache_limit.load(std::memory_order_relaxed),
        static_cast<AdaptiveModeId>(g_config_mode.load(std::memory_order_relaxed)),
        static_cast<AdaptiveModeSelectorKind>(g_selector_kind.load(std::memory_order_relaxed)),
        g_mode_window.load(std::memory_order_relaxed),
        g_mode_cooldown.load(std::memory_order_relaxed),
    };
}

bool adaptive_valid_mode_id(AdaptiveModeId mode) noexcept {
    return adaptive_mode_index(mode) == static_cast<size_t>(static_cast<uint8_t>(mode));
}

size_t adaptive_mode_index(AdaptiveModeId mode) noexcept {
    uint8_t value = static_cast<uint8_t>(mode);
    return value < ADAPTIVE_MODE_COUNT ? value : 0;
}

AdaptiveModeId adaptive_current_mode() noexcept {
    adaptive_runtime_init();
    return static_cast<AdaptiveModeId>(g_active_mode.load(std::memory_order_relaxed));
}

AdaptiveModeId adaptive_previous_mode() noexcept {
    adaptive_runtime_init();
    return static_cast<AdaptiveModeId>(g_previous_mode.load(std::memory_order_relaxed));
}

AdaptiveModeSelectorKind adaptive_selector_kind() noexcept {
    adaptive_runtime_init();
    return static_cast<AdaptiveModeSelectorKind>(g_selector_kind.load(std::memory_order_relaxed));
}

uint64_t adaptive_selector_window_counter() noexcept {
    return g_selector_counter.fetch_add(1, std::memory_order_relaxed) + 1;
}

uint64_t adaptive_selector_cooldown_until() noexcept {
    return g_cooldown_until.load(std::memory_order_relaxed);
}

void adaptive_selector_set_cooldown_until(uint64_t value) noexcept {
    g_cooldown_until.store(value, std::memory_order_relaxed);
}

void adaptive_activate_mode(AdaptiveModeId mode) noexcept {
    if (!adaptive_valid_mode_id(mode)) return;
    adaptive_runtime_init();
    AdaptiveModeId old = adaptive_current_mode();
    if (old == mode) return;
    adaptive_mode_policy(old).on_retire();
    g_mode_states[adaptive_mode_index(old)].store(static_cast<uint8_t>(AdaptiveModeState::Retired),
                                                  std::memory_order_relaxed);
    g_previous_mode.store(static_cast<uint8_t>(old), std::memory_order_relaxed);
    g_active_mode.store(static_cast<uint8_t>(mode), std::memory_order_relaxed);
    g_mode_states[adaptive_mode_index(mode)].store(static_cast<uint8_t>(AdaptiveModeState::Active),
                                                   std::memory_order_relaxed);
    adaptive_mode_policy(mode).on_activate();
    adaptive_telemetry_on_mode_switch();
}

void adaptive_set_mode(AdaptiveModeId mode) noexcept {
    adaptive_activate_mode(mode);
}

uint64_t adaptive_thread_token() noexcept {
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pthread_self()));
}

uint64_t adaptive_next_alloc_epoch() noexcept {
    return g_alloc_epoch.fetch_add(1, std::memory_order_relaxed) + 1;
}

uint64_t adaptive_retired_mode_count() noexcept {
    adaptive_runtime_init();
    uint64_t count = 0;
    for (size_t i = 0; i < ADAPTIVE_MODE_COUNT; ++i) {
        if (g_mode_states[i].load(std::memory_order_relaxed) ==
            static_cast<uint8_t>(AdaptiveModeState::Retired)) {
            count++;
        }
    }
    return count;
}

AdaptiveModeState adaptive_mode_state(AdaptiveModeId mode) noexcept {
    adaptive_runtime_init();
    return static_cast<AdaptiveModeState>(
        g_mode_states[adaptive_mode_index(mode)].load(std::memory_order_relaxed));
}

} // namespace my_ptmalloc
