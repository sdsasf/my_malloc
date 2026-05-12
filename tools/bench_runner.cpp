#include "strategy_loader.h"
#include "telemetry_server.h"
#include "my_ptmalloc/adaptive_allocator.h"
#include "my_ptmalloc/adaptive_selector.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <fstream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <sys/resource.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>

using my_ptmalloc::StrategyDescriptor;
using my_ptmalloc::tools::LoadedStrategy;
using my_ptmalloc::tools::load_strategy;
using my_ptmalloc::tools::unload_strategy;

struct BenchResult {
    std::string name;
    double ms;
    size_t ops;
    size_t peak_rss_kb;
    std::string extra_json;
};

struct BenchConfig {
    std::string strategy = "hybrid";
    std::string profile = "micro";
    std::vector<std::string> benches;
    bool json = false;
    size_t size = 64;
    size_t min_size = 16;
    size_t max_size = 8192;
    int iters = 1000000;
    int random_iters = 200000;
    int slots = 4096;
    int batch = 512;
    int rounds = 1000;
    int threads = 4;
    int repeats = 1;
    unsigned seed = 12345;
    std::string workload_template = "adaptive_mix";
    std::string workload_config;
    bool workload_realtime = false;
    int phase_ms = 10000;
    int target_ops_per_sec = 50000;
    int phase_repeat = 1;
    bool payload_validation = false;
    int payload_validation_rate = 64;
    int telemetry_port = 0;
    int telemetry_hold_ms = 300000;
};

struct RepeatSummary {
    double mean;
    double median;
    double p95;
    double stddev;
    double min;
    double max;
};

static double now_ms() {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static size_t peak_rss_kb() {
    rusage ru{};
    getrusage(RUSAGE_SELF, &ru);
    return static_cast<size_t>(ru.ru_maxrss);
}

struct WorkloadSnapshot {
    char strategy[32]{};
    char benchmark[64]{};
    char template_name[64]{};
    char phase[64]{};
    int phase_index = 0;
    int phase_count = 0;
    uint64_t ops = 0;
    uint64_t allocs = 0;
    uint64_t frees = 0;
    uint64_t reallocs = 0;
    uint64_t remote_frees = 0;
    uint64_t requested_bytes = 0;
    uint64_t live_objects = 0;
    uint64_t live_bytes = 0;
    uint64_t peak_live_bytes = 0;
    uint64_t validation_checks = 0;
    uint64_t validation_errors = 0;
    double elapsed_ms = 0.0;
    double phase_elapsed_ms = 0.0;
    double phase_duration_ms = 0.0;
    double phase_progress = 0.0;
    bool running = false;
};

std::mutex g_workload_snapshot_mutex;
WorkloadSnapshot g_workload_snapshot;
double g_workload_start_ms = 0.0;

static void set_workload_identity(const char* strategy,
                                  const char* benchmark,
                                  const char* template_name) {
    std::lock_guard<std::mutex> lock(g_workload_snapshot_mutex);
    std::snprintf(g_workload_snapshot.strategy, sizeof(g_workload_snapshot.strategy), "%s", strategy);
    std::snprintf(g_workload_snapshot.benchmark, sizeof(g_workload_snapshot.benchmark), "%s", benchmark);
    std::snprintf(g_workload_snapshot.template_name, sizeof(g_workload_snapshot.template_name), "%s", template_name);
}

static void update_workload_snapshot(const char* phase,
                                     int phase_index,
                                     int phase_count,
                                     uint64_t ops,
                                     uint64_t allocs,
                                     uint64_t frees,
                                     uint64_t reallocs,
                                     uint64_t remote_frees,
                                     uint64_t requested_bytes,
                                     uint64_t live_objects,
                                     uint64_t live_bytes,
                                     uint64_t peak_live_bytes,
                                     uint64_t validation_checks,
                                     uint64_t validation_errors,
                                     double phase_elapsed_ms,
                                     double phase_duration_ms,
                                     bool running) {
    std::lock_guard<std::mutex> lock(g_workload_snapshot_mutex);
    std::snprintf(g_workload_snapshot.phase, sizeof(g_workload_snapshot.phase), "%s", phase);
    g_workload_snapshot.phase_index = phase_index;
    g_workload_snapshot.phase_count = phase_count;
    g_workload_snapshot.ops = ops;
    g_workload_snapshot.allocs = allocs;
    g_workload_snapshot.frees = frees;
    g_workload_snapshot.reallocs = reallocs;
    g_workload_snapshot.remote_frees = remote_frees;
    g_workload_snapshot.requested_bytes = requested_bytes;
    g_workload_snapshot.live_objects = live_objects;
    g_workload_snapshot.live_bytes = live_bytes;
    g_workload_snapshot.peak_live_bytes = peak_live_bytes;
    g_workload_snapshot.validation_checks = validation_checks;
    g_workload_snapshot.validation_errors = validation_errors;
    g_workload_snapshot.elapsed_ms = g_workload_start_ms > 0.0 ? now_ms() - g_workload_start_ms : 0.0;
    g_workload_snapshot.phase_elapsed_ms = phase_elapsed_ms;
    g_workload_snapshot.phase_duration_ms = phase_duration_ms;
    g_workload_snapshot.phase_progress = phase_duration_ms > 0.0
        ? std::min(1.0, phase_elapsed_ms / phase_duration_ms)
        : 0.0;
    g_workload_snapshot.running = running;
}

static WorkloadSnapshot workload_snapshot_copy() {
    std::lock_guard<std::mutex> lock(g_workload_snapshot_mutex);
    return g_workload_snapshot;
}

static void append_selector_event_json(std::ostringstream& os,
                                       const my_ptmalloc::AdaptiveSelectorEvent& e) {
    const auto& f = e.features;
    os << "{\"sequence\":" << e.sequence
       << ",\"previous_mode\":\"" << my_ptmalloc::adaptive_mode_name(e.previous_mode)
       << "\",\"current_mode\":\"" << my_ptmalloc::adaptive_mode_name(e.current_mode)
       << "\",\"candidate_mode\":\"" << my_ptmalloc::adaptive_mode_name(e.candidate_mode)
       << "\",\"switched\":" << (e.switched ? "true" : "false")
       << ",\"reason\":\"" << e.reason
       << "\",\"selector_backend\":\"" << e.selector_backend
       << "\",\"rule_candidate\":\"" << my_ptmalloc::adaptive_mode_name(e.rule_candidate)
       << "\",\"model_candidate\":\"" << my_ptmalloc::adaptive_mode_name(e.model_candidate)
       << "\",\"model_confidence\":" << e.model_confidence
       << ",\"large_bytes_ratio\":" << f.large_bytes_ratio
       << ",\"remote_free_ratio\":" << f.remote_free_ratio
       << ",\"mapped_live_ratio\":" << f.mapped_live_ratio
       << ",\"size_entropy\":" << f.size_entropy
       << ",\"fragmentation_estimate\":" << f.internal_frag_ratio
       << ",\"slow_path_ratio\":" << f.slow_path_ratio
       << ",\"cache_hit_rate\":" << f.cache_hit_rate
       << "}";
}

template <typename T, size_t N>
static void append_array_json(std::ostringstream& os, const char* name, const T (&values)[N]) {
    os << ",\"" << name << "\":[";
    for (size_t i = 0; i < N; ++i) {
        if (i) os << ",";
        os << values[i];
    }
    os << "]";
}

static void touch_bytes(void* p, size_t size, unsigned char value) {
    if (!p) return;
    std::memset(p, value, std::min<size_t>(size, 64));
}

static std::string telemetry_snapshot_json() {
    WorkloadSnapshot w = workload_snapshot_copy();
    std::ostringstream os;
    double ops_sec = w.elapsed_ms > 0.0 ? static_cast<double>(w.ops) / (w.elapsed_ms / 1000.0) : 0.0;
    os << "{\"workload\":{\"strategy\":\"" << w.strategy
       << "\",\"benchmark\":\"" << w.benchmark
       << "\",\"template\":\"" << w.template_name
       << "\",\"phase\":\"" << w.phase
       << "\",\"phase_index\":" << w.phase_index
       << ",\"phase_count\":" << w.phase_count
       << ",\"running\":" << (w.running ? "true" : "false")
       << ",\"elapsed_ms\":" << w.elapsed_ms
       << ",\"ops\":" << w.ops
       << ",\"ops_per_sec\":" << ops_sec
       << ",\"allocs\":" << w.allocs
       << ",\"frees\":" << w.frees
       << ",\"reallocs\":" << w.reallocs
       << ",\"remote_frees\":" << w.remote_frees
       << ",\"requested_bytes\":" << w.requested_bytes
       << ",\"live_objects\":" << w.live_objects
       << ",\"live_bytes\":" << w.live_bytes
       << ",\"peak_live_bytes\":" << w.peak_live_bytes
       << ",\"phase_elapsed_ms\":" << w.phase_elapsed_ms
       << ",\"phase_duration_ms\":" << w.phase_duration_ms
       << ",\"phase_progress\":" << w.phase_progress
       << ",\"peak_rss_kb\":" << peak_rss_kb()
       << "},\"generated\":{\"validation_checks\":" << w.validation_checks
       << ",\"validation_errors\":" << w.validation_errors
       << "}";
    if (std::strcmp(w.strategy, "adaptive") == 0) {
        auto s = my_ptmalloc::adaptive_stats_snapshot();
        os << ",\"adaptive\":{\"current_mode\":\"" << my_ptmalloc::adaptive_mode_name(s.current_mode)
           << "\",\"active_mode\":\"" << my_ptmalloc::adaptive_mode_name(s.active_mode)
           << "\",\"previous_mode\":\"" << my_ptmalloc::adaptive_mode_name(s.previous_mode)
           << "\",\"mode_switches\":" << s.mode_switches
           << ",\"retired_mode_count\":" << s.retired_mode_count
           << ",\"mapped_bytes\":" << s.mapped_bytes
           << ",\"live_bytes\":" << s.live_bytes
           << ",\"mapped_live_ratio\":" << s.mapped_live_ratio
           << ",\"remote_free_ratio\":" << s.remote_free_ratio
           << ",\"size_entropy\":" << s.size_entropy
           << ",\"large_bytes_ratio\":" << s.large_bytes_ratio
           << ",\"fragmentation_estimate\":" << s.fragmentation_estimate
           << ",\"slow_path_ratio\":" << s.slow_path_ratio
           << ",\"double_free_count\":" << s.double_free_count
           << ",\"invalid_free_count\":" << s.invalid_free_count
           << ",\"header_corruption_count\":" << s.header_corruption_count
           << ",\"empty_pages\":" << s.empty_pages
           << ",\"empty_spans\":" << s.empty_spans
           << ",\"released_pages\":" << s.released_pages
           << ",\"released_spans\":" << s.released_spans
           << ",\"release_unmapped_bytes\":" << s.release_unmapped_bytes;
        append_array_json(os, "mode_alloc_count", s.mode_alloc_count);
        append_array_json(os, "mode_free_count", s.mode_free_count);
        append_array_json(os, "mode_live_bytes", s.mode_live_bytes);
        append_array_json(os, "mode_mapped_bytes", s.mode_mapped_bytes);
        uint64_t storage_allocs[3] = {
            s.storage[0].alloc_count,
            s.storage[1].alloc_count,
            s.storage[2].alloc_count,
        };
        uint64_t storage_frees[3] = {
            s.storage[0].free_count,
            s.storage[1].free_count,
            s.storage[2].free_count,
        };
        uint64_t storage_requested[3] = {
            s.storage[0].requested_bytes,
            s.storage[1].requested_bytes,
            s.storage[2].requested_bytes,
        };
        uint64_t storage_usable[3] = {
            s.storage[0].usable_bytes,
            s.storage[1].usable_bytes,
            s.storage[2].usable_bytes,
        };
        uint64_t pool_hits[3] = {
            s.storage[0].pool_hits,
            s.storage[1].pool_hits,
            s.storage[2].pool_hits,
        };
        uint64_t pool_misses[3] = {
            s.storage[0].pool_misses,
            s.storage[1].pool_misses,
            s.storage[2].pool_misses,
        };
        append_array_json(os, "storage_allocs", storage_allocs);
        append_array_json(os, "storage_frees", storage_frees);
        append_array_json(os, "storage_requested_bytes", storage_requested);
        append_array_json(os, "storage_usable_bytes", storage_usable);
        append_array_json(os, "pool_hits", pool_hits);
        append_array_json(os, "pool_misses", pool_misses);
        os << "}";
        my_ptmalloc::AdaptiveSelectorEvent last_event{};
        if (my_ptmalloc::adaptive_selector_last_event(last_event)) {
            os << ",\"selector_last_window\":";
            append_selector_event_json(os, last_event);
        }
        my_ptmalloc::AdaptiveSelectorEvent events[16]{};
        size_t event_count = my_ptmalloc::adaptive_selector_events_snapshot(events, 16);
        os << ",\"selector_events\":[";
        for (size_t i = 0; i < event_count; ++i) {
            if (i) os << ",";
            append_selector_event_json(os, events[i]);
        }
        os << "]";
    }
    os << "}";
    return os.str();
}

static BenchResult same_size(const StrategyDescriptor& s, size_t size, int iters) {
    volatile unsigned char acc = 0;
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0xCD);
        if (p) acc += *static_cast<unsigned char*>(p);
        s.vtable.deallocate(p);
    }
    double end = now_ms();
    char name[64];
    std::snprintf(name, sizeof(name), "same_size_%zu", size);
    return {name, end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult batch_workload(const StrategyDescriptor& s, int batch, int rounds) {
    double start = now_ms();
    for (int r = 0; r < rounds; ++r) {
        std::vector<void*> ptrs(batch);
        for (int i = 0; i < batch; ++i) {
            size_t size = 128 + i % 512;
            ptrs[i] = s.vtable.allocate(size);
            touch_bytes(ptrs[i], size, 0xEF);
        }
        for (void* p : ptrs) s.vtable.deallocate(p);
    }
    double end = now_ms();
    return {"batch", end - start, static_cast<size_t>(batch) * rounds, peak_rss_kb()};
}

static BenchResult random_workload(const StrategyDescriptor& s,
                                   int iters,
                                   int slots,
                                   size_t min_size,
                                   size_t max_size,
                                   unsigned seed) {
    std::vector<void*> ptrs(slots, nullptr);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<size_t> size_dist(min_size, max_size);
    std::uniform_int_distribution<int> action_dist(0, 4);
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        int idx = static_cast<int>(rng() % slots);
        int action = action_dist(rng);
        if (!ptrs[idx] || action <= 2) {
            void* p = s.vtable.allocate(size_dist(rng));
            touch_bytes(p, 16, 0xAB);
            if (ptrs[idx]) s.vtable.deallocate(ptrs[idx]);
            ptrs[idx] = p;
        } else if (action == 3) {
            ptrs[idx] = s.vtable.reallocate(ptrs[idx], size_dist(rng));
        } else {
            s.vtable.deallocate(ptrs[idx]);
            ptrs[idx] = nullptr;
        }
    }
    for (void* p : ptrs) if (p) s.vtable.deallocate(p);
    double end = now_ms();
    return {"random", end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult fragmentation_workload(const StrategyDescriptor& s,
                                          int iters,
                                          int slots,
                                          size_t min_size,
                                          size_t max_size,
                                          unsigned seed) {
    std::vector<void*> ptrs(slots, nullptr);
    std::vector<size_t> sizes(slots, 0);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<size_t> size_dist(min_size, max_size);
    for (int i = 0; i < slots; ++i) {
        sizes[i] = size_dist(rng);
        ptrs[i] = s.vtable.allocate(sizes[i]);
        touch_bytes(ptrs[i], sizes[i], 0xA5);
    }

    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        int idx = static_cast<int>(rng() % slots);
        size_t next_size = size_dist(rng);
        void* next = s.vtable.reallocate(ptrs[idx], next_size);
        if (next) {
            ptrs[idx] = next;
            sizes[idx] = next_size;
            touch_bytes(ptrs[idx], sizes[idx], 0x5A);
        } else {
            s.vtable.deallocate(ptrs[idx]);
            ptrs[idx] = nullptr;
            sizes[idx] = 0;
        }
    }
    for (void* p : ptrs) if (p) s.vtable.deallocate(p);
    double end = now_ms();
    return {"fragmentation", end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult cross_thread_free_workload(const StrategyDescriptor& s,
                                              int threads,
                                              int per_thread,
                                              size_t min_size,
                                              size_t max_size,
                                              unsigned seed) {
    std::vector<std::vector<void*>> ptrs(static_cast<size_t>(threads));
    for (auto& v : ptrs) v.resize(static_cast<size_t>(per_thread), nullptr);

    double start = now_ms();
    std::vector<std::thread> producers;
    for (int t = 0; t < threads; ++t) {
        producers.emplace_back([&, t] {
            std::mt19937 rng(seed + static_cast<unsigned>(t));
            std::uniform_int_distribution<size_t> size_dist(min_size, max_size);
            for (int i = 0; i < per_thread; ++i) {
                size_t size = size_dist(rng);
                ptrs[static_cast<size_t>(t)][static_cast<size_t>(i)] = s.vtable.allocate(size);
                touch_bytes(ptrs[static_cast<size_t>(t)][static_cast<size_t>(i)], size, 0xC3);
            }
        });
    }
    for (auto& th : producers) th.join();

    std::vector<std::thread> consumers;
    for (int t = 0; t < threads; ++t) {
        consumers.emplace_back([&, t] {
            int victim = (t + threads - 1) % threads;
            for (void* p : ptrs[static_cast<size_t>(victim)]) {
                s.vtable.deallocate(p);
            }
        });
    }
    for (auto& th : consumers) th.join();
    double end = now_ms();
    return {"cross_thread_free", end - start, static_cast<size_t>(threads) * per_thread * 2, peak_rss_kb()};
}

static BenchResult latency_sample_workload(const StrategyDescriptor& s,
                                           size_t size,
                                           int iters,
                                           int sample_every) {
    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(iters / sample_every + 1));
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        double op_start = 0;
        if (i % sample_every == 0) op_start = now_ms();
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x9C);
        s.vtable.deallocate(p);
        if (i % sample_every == 0) samples.push_back(now_ms() - op_start);
    }
    double end = now_ms();
    std::sort(samples.begin(), samples.end());
    double p99_us = samples.empty() ? 0 : samples[static_cast<size_t>(samples.size() * 99 / 100)] * 1000.0;
    char name[96];
    std::snprintf(name, sizeof(name), "latency_sample_%zu_p99us_%.2f", size, p99_us);
    return {name, end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult phase_changing_workload(const StrategyDescriptor& s,
                                           int iters,
                                           int slots,
                                           unsigned seed) {
    std::vector<void*> ptrs(static_cast<size_t>(slots), nullptr);
    std::mt19937 rng(seed);
    double start = now_ms();

    for (int i = 0; i < iters; ++i) {
        void* p = s.vtable.allocate(32 + static_cast<size_t>(i % 8) * 16);
        touch_bytes(p, 64, 0x11);
        s.vtable.deallocate(p);
    }
    std::uniform_int_distribution<size_t> medium_size(1024, 64 * 1024);
    for (int i = 0; i < iters / 2; ++i) {
        int idx = static_cast<int>(rng() % slots);
        if (ptrs[idx]) s.vtable.deallocate(ptrs[idx]);
        ptrs[idx] = s.vtable.allocate(medium_size(rng));
        touch_bytes(ptrs[idx], 128, 0x22);
    }
    for (int i = 0; i < std::max(1, iters / 32); ++i) {
        void* p = s.vtable.allocate(128 * 1024 + static_cast<size_t>(i % 8) * 64 * 1024);
        touch_bytes(p, 256, 0x33);
        s.vtable.deallocate(p);
    }
    for (int i = 0; i < slots; ++i) {
        if (!ptrs[static_cast<size_t>(i)]) {
            ptrs[static_cast<size_t>(i)] = s.vtable.allocate(256 + static_cast<size_t>(i % 64) * 16);
        }
    }
    for (void* p : ptrs) if (p) s.vtable.deallocate(p);

    double end = now_ms();
    return {"phase_changing", end - start, static_cast<size_t>(iters * 2 + slots), peak_rss_kb()};
}

static BenchResult large_streaming_workload(const StrategyDescriptor& s, int iters) {
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        size_t size = 128 * 1024 + static_cast<size_t>(i % 16) * 64 * 1024;
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x42);
        s.vtable.deallocate(p);
    }
    double end = now_ms();
    return {"large_streaming", end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult debug_safety_workload(const StrategyDescriptor& s, int iters, bool exercise_invalid_free) {
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        void* p = s.vtable.allocate(64 + static_cast<size_t>(i % 8) * 16);
        touch_bytes(p, 64, 0xD5);
        s.vtable.deallocate(p);
        if (exercise_invalid_free && i % 64 == 0) {
            s.vtable.deallocate(p);
        }
    }
    double end = now_ms();
    return {"debug_safety", end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

enum class GeneratedPhaseKind {
    SmallChurn,
    FragmentationDrift,
    RemoteFree,
    LargeBurst,
    PeakRelease,
    LatencyLoop,

    // Diagnostic workloads for selector validation.
    TinyCacheChurn,
    RemoteDominant,
    MediumFragStrong,
    RssDecayHold,
};

struct GeneratedPhase {
    std::string name;
    GeneratedPhaseKind kind;
    int weight;
    int duration_ms = 0;
    int target_ops_per_sec = 0;
    int slots = 0;
    int threads = 0;
    size_t min_size = 0;
    size_t max_size = 0;
};

struct GeneratedMetrics {
    uint64_t ops = 0;
    uint64_t allocs = 0;
    uint64_t frees = 0;
    uint64_t reallocs = 0;
    uint64_t remote_frees = 0;
    uint64_t requested_bytes = 0;
    uint64_t live_objects = 0;
    uint64_t live_bytes = 0;
    uint64_t peak_live_bytes = 0;
    uint64_t validation_checks = 0;
    uint64_t validation_errors = 0;
};

struct PayloadRecord {
    void* ptr = nullptr;
    size_t size = 0;
    uint64_t id = 0;
    unsigned char seed = 0;
};

static void payload_write(void* ptr, size_t size, unsigned char seed) {
    if (!ptr) return;
    unsigned char* bytes = static_cast<unsigned char*>(ptr);
    size_t n = std::min<size_t>(size, 64);
    for (size_t i = 0; i < n; ++i) bytes[i] = static_cast<unsigned char>(seed + i * 17u);
}

static bool payload_check(void* ptr, size_t size, unsigned char seed) {
    if (!ptr) return true;
    unsigned char* bytes = static_cast<unsigned char*>(ptr);
    size_t n = std::min<size_t>(size, 64);
    for (size_t i = 0; i < n; ++i) {
        if (bytes[i] != static_cast<unsigned char>(seed + i * 17u)) return false;
    }
    return true;
}

static bool should_validate(const BenchConfig& cfg, const GeneratedMetrics& m) {
    return cfg.payload_validation &&
           cfg.payload_validation_rate > 0 &&
           (m.allocs % static_cast<uint64_t>(cfg.payload_validation_rate)) == 0;
}

static void validation_check_record(const BenchConfig& cfg,
                                    GeneratedMetrics& m,
                                    const PayloadRecord& rec) {
    if (!rec.ptr || !should_validate(cfg, m)) return;
    m.validation_checks++;
    if (!payload_check(rec.ptr, rec.size, rec.seed)) m.validation_errors++;
}

static void metrics_alloc(GeneratedMetrics& m, size_t size) {
    m.ops++;
    m.allocs++;
    m.requested_bytes += size;
    m.live_objects++;
    m.live_bytes += size;
    m.peak_live_bytes = std::max(m.peak_live_bytes, m.live_bytes);
}

static void metrics_free(GeneratedMetrics& m, size_t size, bool remote = false) {
    m.ops++;
    m.frees++;
    if (remote) m.remote_frees++;
    if (m.live_objects > 0) m.live_objects--;
    m.live_bytes = m.live_bytes > size ? m.live_bytes - size : 0;
}

static void metrics_realloc(GeneratedMetrics& m, size_t old_size, size_t new_size) {
    m.ops++;
    m.reallocs++;
    m.requested_bytes += new_size;
    m.live_bytes = m.live_bytes > old_size ? m.live_bytes - old_size : 0;
    m.live_bytes += new_size;
    m.peak_live_bytes = std::max(m.peak_live_bytes, m.live_bytes);
}

static std::vector<GeneratedPhase> generated_template(const std::string& name) {
    if (name == "tiny_cache_churn") {
        return {{"tiny_cache_churn", GeneratedPhaseKind::TinyCacheChurn, 10}};
    }
    if (name == "remote_dominant") {
        return {{"remote_dominant", GeneratedPhaseKind::RemoteDominant, 10}};
    }
    if (name == "medium_frag_strong") {
        return {{"medium_frag_strong", GeneratedPhaseKind::MediumFragStrong, 10}};
    }
    if (name == "rss_decay_hold") {
        return {{"rss_decay_hold", GeneratedPhaseKind::RssDecayHold, 10}};
    }

    if (name == "throughput_churn") {
        return {{"small_churn", GeneratedPhaseKind::SmallChurn, 8},
                {"latency_loop", GeneratedPhaseKind::LatencyLoop, 2}};
    }
    if (name == "remote_queue") {
        return {{"small_churn", GeneratedPhaseKind::SmallChurn, 2},
                {"remote_free", GeneratedPhaseKind::RemoteFree, 8}};
    }
    if (name == "large_burst") {
        return {{"small_churn", GeneratedPhaseKind::SmallChurn, 2},
                {"large_burst", GeneratedPhaseKind::LargeBurst, 8}};
    }
    if (name == "rss_peak_release") {
        return {{"peak_release", GeneratedPhaseKind::PeakRelease, 8},
                {"small_churn", GeneratedPhaseKind::SmallChurn, 2}};
    }
    if (name == "fragmentation_drift") {
        return {{"fragmentation_drift", GeneratedPhaseKind::FragmentationDrift, 10}};
    }
    if (name == "latency_loop") {
        return {{"latency_loop", GeneratedPhaseKind::LatencyLoop, 10}};
    }
    return {{"small_churn", GeneratedPhaseKind::SmallChurn, 2},
            {"fragmentation_drift", GeneratedPhaseKind::FragmentationDrift, 3},
            {"remote_free", GeneratedPhaseKind::RemoteFree, 2},
            {"large_burst", GeneratedPhaseKind::LargeBurst, 2},
            {"peak_release", GeneratedPhaseKind::PeakRelease, 2},
            {"latency_loop", GeneratedPhaseKind::LatencyLoop, 1}};
}

static GeneratedPhaseKind generated_kind_from_name(const std::string& name) {
    if (name.find("tiny_cache") != std::string::npos ||
        name.find("small_cache") != std::string::npos) {
        return GeneratedPhaseKind::TinyCacheChurn;
    }
    if (name.find("remote_dominant") != std::string::npos) {
        return GeneratedPhaseKind::RemoteDominant;
    }
    if (name.find("medium_frag") != std::string::npos ||
        name.find("frag_strong") != std::string::npos) {
        return GeneratedPhaseKind::MediumFragStrong;
    }
    if (name.find("rss_decay") != std::string::npos) {
        return GeneratedPhaseKind::RssDecayHold;
    }

    if (name.find("fragmentation") != std::string::npos) return GeneratedPhaseKind::FragmentationDrift;
    if (name.find("remote") != std::string::npos || name.find("producer") != std::string::npos) {
        return GeneratedPhaseKind::RemoteFree;
    }
    if (name.find("large") != std::string::npos) return GeneratedPhaseKind::LargeBurst;
    if (name.find("peak") != std::string::npos || name.find("rss") != std::string::npos) {
        return GeneratedPhaseKind::PeakRelease;
    }
    if (name.find("latency") != std::string::npos) return GeneratedPhaseKind::LatencyLoop;
    return GeneratedPhaseKind::SmallChurn;
}

static bool json_find_string(const std::string& object, const char* key, std::string& out) {
    std::string needle = std::string("\"") + key + "\"";
    size_t pos = object.find(needle);
    if (pos == std::string::npos) return false;
    pos = object.find(':', pos);
    if (pos == std::string::npos) return false;
    pos = object.find('"', pos);
    if (pos == std::string::npos) return false;
    size_t end = object.find('"', pos + 1);
    if (end == std::string::npos) return false;
    out = object.substr(pos + 1, end - pos - 1);
    return true;
}

static bool json_find_int(const std::string& object, const char* key, int& out) {
    std::string needle = std::string("\"") + key + "\"";
    size_t pos = object.find(needle);
    if (pos == std::string::npos) return false;
    pos = object.find(':', pos);
    if (pos == std::string::npos) return false;
    char* end = nullptr;
    long v = std::strtol(object.c_str() + pos + 1, &end, 10);
    if (end == object.c_str() + pos + 1) return false;
    out = static_cast<int>(v);
    return true;
}

static bool json_find_size(const std::string& object, const char* key, size_t& out) {
    int tmp = 0;
    if (!json_find_int(object, key, tmp) || tmp <= 0) return false;
    out = static_cast<size_t>(tmp);
    return true;
}

static std::vector<GeneratedPhase> load_generated_config(const std::string& path,
                                                         std::string& workload_name) {
    std::ifstream in(path);
    if (!in) return {};
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::string configured_name;
    if (json_find_string(text, "name", configured_name)) workload_name = configured_name;
    std::vector<GeneratedPhase> phases;
    size_t array_pos = text.find("\"phases\"");
    if (array_pos == std::string::npos) return phases;
    size_t pos = text.find('{', array_pos);
    while (pos != std::string::npos) {
        int depth = 0;
        size_t end = pos;
        for (; end < text.size(); ++end) {
            if (text[end] == '{') depth++;
            if (text[end] == '}') {
                depth--;
                if (depth == 0) break;
            }
        }
        if (end >= text.size()) break;
        std::string object = text.substr(pos, end - pos + 1);
        std::string name;
        std::string kind;
        json_find_string(object, "name", name);
        json_find_string(object, "kind", kind);
        if (name.empty()) name = kind.empty() ? "configured_phase" : kind;
        GeneratedPhase phase{name, generated_kind_from_name(kind.empty() ? name : kind), 1};
        json_find_int(object, "weight", phase.weight);
        json_find_int(object, "duration_ms", phase.duration_ms);
        json_find_int(object, "target_ops_per_sec", phase.target_ops_per_sec);
        json_find_int(object, "slots", phase.slots);
        json_find_int(object, "threads", phase.threads);
        json_find_size(object, "min_size", phase.min_size);
        json_find_size(object, "max_size", phase.max_size);
        phases.push_back(phase);
        pos = text.find('{', end + 1);
        size_t close_array = text.find(']', end + 1);
        if (close_array != std::string::npos && close_array < pos) break;
    }
    return phases;
}

static void publish_generated(const GeneratedPhase& phase,
                              int phase_index,
                              int phase_count,
                              const GeneratedMetrics& m,
                              double phase_elapsed_ms = 0.0,
                              double phase_duration_ms = 0.0,
                              bool running = true) {
    update_workload_snapshot(phase.name.c_str(), phase_index, phase_count, m.ops, m.allocs, m.frees,
                             m.reallocs, m.remote_frees, m.requested_bytes, m.live_objects,
                             m.live_bytes, m.peak_live_bytes, m.validation_checks,
                             m.validation_errors, phase_elapsed_ms,
                             phase_duration_ms, running);
}

static void generated_small_churn(const StrategyDescriptor& s,
                                  int ops,
                                  GeneratedMetrics& m,
                                  const GeneratedPhase& phase,
                                  int phase_index,
                                  int phase_count) {
    for (int i = 0; i < ops; ++i) {
        size_t size = 32 + static_cast<size_t>(i % 8) * 16;
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x21);
        metrics_alloc(m, size);
        s.vtable.deallocate(p);
        metrics_free(m, size);
        if ((i & 1023) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
}

static void generated_small_churn_chunk(const StrategyDescriptor& s,
                                        int ops,
                                        const BenchConfig& cfg,
                                        GeneratedMetrics& m) {
    for (int i = 0; i < ops; ++i) {
        size_t size = 32 + static_cast<size_t>((m.ops + static_cast<uint64_t>(i)) % 8) * 16;
        void* p = s.vtable.allocate(size);
        unsigned char seed = static_cast<unsigned char>(0x31 + (m.allocs & 31));
        payload_write(p, size, seed);
        metrics_alloc(m, size);
        PayloadRecord rec{p, size, m.allocs, seed};
        validation_check_record(cfg, m, rec);
        s.vtable.deallocate(p);
        metrics_free(m, size);
    }
}

static void generated_latency_loop(const StrategyDescriptor& s,
                                   int ops,
                                   GeneratedMetrics& m,
                                   const GeneratedPhase& phase,
                                   int phase_index,
                                   int phase_count) {
    for (int i = 0; i < ops; ++i) {
        void* p = s.vtable.allocate(64);
        touch_bytes(p, 64, 0x22);
        metrics_alloc(m, 64);
        s.vtable.deallocate(p);
        metrics_free(m, 64);
        if ((i & 2047) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
}

static void generated_latency_loop_chunk(const StrategyDescriptor& s,
                                         int ops,
                                         const BenchConfig& cfg,
                                         GeneratedMetrics& m) {
    for (int i = 0; i < ops; ++i) {
        void* p = s.vtable.allocate(64);
        unsigned char seed = static_cast<unsigned char>(0x32 + (m.allocs & 31));
        payload_write(p, 64, seed);
        metrics_alloc(m, 64);
        PayloadRecord rec{p, 64, m.allocs, seed};
        validation_check_record(cfg, m, rec);
        s.vtable.deallocate(p);
        metrics_free(m, 64);
    }
}

static void generated_fragmentation(const StrategyDescriptor& s,
                                    int ops,
                                    int slots,
                                    unsigned seed,
                                    GeneratedMetrics& m,
                                    const GeneratedPhase& phase,
                                    int phase_index,
                                    int phase_count) {
    slots = std::max(16, slots);
    std::vector<void*> ptrs(static_cast<size_t>(slots), nullptr);
    std::vector<size_t> sizes(static_cast<size_t>(slots), 0);
    std::mt19937 rng(seed + 17);
    std::uniform_int_distribution<size_t> size_dist(128, 96 * 1024);
    for (int i = 0; i < slots; ++i) {
        sizes[static_cast<size_t>(i)] = size_dist(rng);
        ptrs[static_cast<size_t>(i)] = s.vtable.allocate(sizes[static_cast<size_t>(i)]);
        touch_bytes(ptrs[static_cast<size_t>(i)], sizes[static_cast<size_t>(i)], 0x23);
        metrics_alloc(m, sizes[static_cast<size_t>(i)]);
    }
    for (int i = 0; i < ops; ++i) {
        size_t idx = static_cast<size_t>(rng() % static_cast<unsigned>(slots));
        size_t old_size = sizes[idx];
        size_t next_size = size_dist(rng);
        void* next = s.vtable.reallocate(ptrs[idx], next_size);
        if (next) {
            ptrs[idx] = next;
            sizes[idx] = next_size;
            touch_bytes(next, next_size, 0x24);
            metrics_realloc(m, old_size, next_size);
        }
        if ((i & 511) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
    for (int i = 0; i < slots; ++i) {
        if (ptrs[static_cast<size_t>(i)]) {
            s.vtable.deallocate(ptrs[static_cast<size_t>(i)]);
            metrics_free(m, sizes[static_cast<size_t>(i)]);
        }
    }
}

static void generated_large_burst(const StrategyDescriptor& s,
                                  int ops,
                                  GeneratedMetrics& m,
                                  const GeneratedPhase& phase,
                                  int phase_index,
                                  int phase_count) {
    for (int i = 0; i < ops; ++i) {
        size_t size = 64 * 1024 + static_cast<size_t>(i % 16) * 64 * 1024;
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x25);
        metrics_alloc(m, size);
        s.vtable.deallocate(p);
        metrics_free(m, size);
        if ((i & 127) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
}

static void generated_large_burst_chunk(const StrategyDescriptor& s,
                                        int ops,
                                        const BenchConfig& cfg,
                                        GeneratedMetrics& m) {
    for (int i = 0; i < ops; ++i) {
        size_t size = 64 * 1024 + static_cast<size_t>((m.allocs + static_cast<uint64_t>(i)) % 16) * 64 * 1024;
        void* p = s.vtable.allocate(size);
        unsigned char seed = static_cast<unsigned char>(0x35 + (m.allocs & 31));
        payload_write(p, size, seed);
        metrics_alloc(m, size);
        PayloadRecord rec{p, size, m.allocs, seed};
        validation_check_record(cfg, m, rec);
        s.vtable.deallocate(p);
        metrics_free(m, size);
    }
}

static void generated_peak_release(const StrategyDescriptor& s,
                                   int ops,
                                   int slots,
                                   GeneratedMetrics& m,
                                   const GeneratedPhase& phase,
                                   int phase_index,
                                   int phase_count) {
    slots = std::max(16, std::min(slots, ops));
    std::vector<void*> ptrs(static_cast<size_t>(slots), nullptr);
    std::vector<size_t> sizes(static_cast<size_t>(slots), 0);
    for (int i = 0; i < slots; ++i) {
        sizes[static_cast<size_t>(i)] = 1024 + static_cast<size_t>(i % 64) * 1024;
        ptrs[static_cast<size_t>(i)] = s.vtable.allocate(sizes[static_cast<size_t>(i)]);
        touch_bytes(ptrs[static_cast<size_t>(i)], sizes[static_cast<size_t>(i)], 0x26);
        metrics_alloc(m, sizes[static_cast<size_t>(i)]);
        if ((i & 255) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
    for (int i = 0; i < slots; ++i) {
        s.vtable.deallocate(ptrs[static_cast<size_t>(i)]);
        metrics_free(m, sizes[static_cast<size_t>(i)]);
        if ((i & 255) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
}

static void generated_remote_free(const StrategyDescriptor& s,
                                  int ops,
                                  int threads,
                                  unsigned seed,
                                  GeneratedMetrics& m,
                                  const GeneratedPhase& phase,
                                  int phase_index,
                                  int phase_count) {
    threads = std::max(2, threads);
    int per_thread = std::max(1, ops / threads);
    std::vector<std::vector<void*>> ptrs(static_cast<size_t>(threads));
    std::vector<std::vector<size_t>> sizes(static_cast<size_t>(threads));
    for (int t = 0; t < threads; ++t) {
        ptrs[static_cast<size_t>(t)].resize(static_cast<size_t>(per_thread), nullptr);
        sizes[static_cast<size_t>(t)].resize(static_cast<size_t>(per_thread), 0);
    }
    std::vector<GeneratedMetrics> local(static_cast<size_t>(threads));
    std::vector<std::thread> producers;
    for (int t = 0; t < threads; ++t) {
        producers.emplace_back([&, t] {
            std::mt19937 rng(seed + static_cast<unsigned>(t) * 131);
            std::uniform_int_distribution<size_t> size_dist(64, 4096);
            for (int i = 0; i < per_thread; ++i) {
                size_t size = size_dist(rng);
                sizes[static_cast<size_t>(t)][static_cast<size_t>(i)] = size;
                ptrs[static_cast<size_t>(t)][static_cast<size_t>(i)] = s.vtable.allocate(size);
                touch_bytes(ptrs[static_cast<size_t>(t)][static_cast<size_t>(i)], size, 0x27);
                metrics_alloc(local[static_cast<size_t>(t)], size);
            }
        });
    }
    for (auto& th : producers) th.join();
    for (const auto& lm : local) {
        m.ops += lm.ops;
        m.allocs += lm.allocs;
        m.requested_bytes += lm.requested_bytes;
        m.live_objects += lm.live_objects;
        m.live_bytes += lm.live_bytes;
        m.peak_live_bytes = std::max(m.peak_live_bytes, m.live_bytes);
    }
    publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);

    std::vector<std::thread> consumers;
    for (int t = 0; t < threads; ++t) {
        consumers.emplace_back([&, t] {
            int victim = (t + threads - 1) % threads;
            for (int i = 0; i < per_thread; ++i) {
                s.vtable.deallocate(ptrs[static_cast<size_t>(victim)][static_cast<size_t>(i)]);
            }
        });
    }
    for (auto& th : consumers) th.join();
    for (int t = 0; t < threads; ++t) {
        for (int i = 0; i < per_thread; ++i) {
            metrics_free(m, sizes[static_cast<size_t>(t)][static_cast<size_t>(i)], true);
        }
    }
    publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
}

static std::string generated_extra_json(const std::string& templ, const GeneratedMetrics& m) {
    char buf[512];
    std::snprintf(buf, sizeof(buf),
                  "\"generated\":{\"template\":\"%s\",\"allocs\":%llu,\"frees\":%llu,"
                  "\"reallocs\":%llu,\"remote_frees\":%llu,\"requested_bytes\":%llu,"
                  "\"peak_live_bytes\":%llu,\"validation_checks\":%llu,"
                  "\"validation_errors\":%llu}",
                  templ.c_str(),
                  static_cast<unsigned long long>(m.allocs),
                  static_cast<unsigned long long>(m.frees),
                  static_cast<unsigned long long>(m.reallocs),
                  static_cast<unsigned long long>(m.remote_frees),
                  static_cast<unsigned long long>(m.requested_bytes),
                  static_cast<unsigned long long>(m.peak_live_bytes),
                  static_cast<unsigned long long>(m.validation_checks),
                  static_cast<unsigned long long>(m.validation_errors));
    return buf;
}

struct RealtimePhaseState {
    std::vector<void*> ptrs;
    std::vector<size_t> sizes;
    std::mt19937 rng;
    bool initialized = false;

    explicit RealtimePhaseState(unsigned seed) : rng(seed) {}
};

static void generated_fragmentation_realtime_chunk(const StrategyDescriptor& s,
                                                   int ops,
                                                   int slots,
                                                   GeneratedMetrics& m,
                                                   RealtimePhaseState& state) {
    slots = std::max(16, slots);
    if (!state.initialized) {
        state.ptrs.assign(static_cast<size_t>(slots), nullptr);
        state.sizes.assign(static_cast<size_t>(slots), 0);
        std::uniform_int_distribution<size_t> size_dist(128, 96 * 1024);
        for (int i = 0; i < slots; ++i) {
            size_t size = size_dist(state.rng);
            state.sizes[static_cast<size_t>(i)] = size;
            state.ptrs[static_cast<size_t>(i)] = s.vtable.allocate(size);
            touch_bytes(state.ptrs[static_cast<size_t>(i)], size, 0x41);
            metrics_alloc(m, size);
        }
        state.initialized = true;
    }
    std::uniform_int_distribution<size_t> size_dist(128, 96 * 1024);
    for (int i = 0; i < ops; ++i) {
        size_t idx = static_cast<size_t>(state.rng() % state.ptrs.size());
        size_t old_size = state.sizes[idx];
        size_t next_size = size_dist(state.rng);
        void* next = s.vtable.reallocate(state.ptrs[idx], next_size);
        if (next) {
            state.ptrs[idx] = next;
            state.sizes[idx] = next_size;
            touch_bytes(next, next_size, 0x42);
            metrics_realloc(m, old_size, next_size);
        }
    }
}

static void generated_peak_release_realtime_chunk(const StrategyDescriptor& s,
                                                  int ops,
                                                  int slots,
                                                  GeneratedMetrics& m,
                                                  RealtimePhaseState& state) {
    slots = std::max(16, slots);
    if (!state.initialized) {
        state.ptrs.assign(static_cast<size_t>(slots), nullptr);
        state.sizes.assign(static_cast<size_t>(slots), 0);
        state.initialized = true;
    }
    for (int i = 0; i < ops; ++i) {
        size_t idx = static_cast<size_t>((m.ops + static_cast<uint64_t>(i)) % state.ptrs.size());
        if (!state.ptrs[idx]) {
            size_t size = 1024 + (idx % 64) * 1024;
            state.sizes[idx] = size;
            state.ptrs[idx] = s.vtable.allocate(size);
            touch_bytes(state.ptrs[idx], size, 0x43);
            metrics_alloc(m, size);
        } else {
            s.vtable.deallocate(state.ptrs[idx]);
            metrics_free(m, state.sizes[idx]);
            state.ptrs[idx] = nullptr;
            state.sizes[idx] = 0;
        }
    }
}

static void generated_remote_realtime_chunk(const StrategyDescriptor& s,
                                            int ops,
                                            int threads,
                                            GeneratedMetrics& m,
                                            RealtimePhaseState& state) {
    threads = std::max(2, threads);
    int count = std::max(1, ops / 2);
    std::vector<void*> ptrs(static_cast<size_t>(count), nullptr);
    std::vector<size_t> sizes(static_cast<size_t>(count), 0);
    std::uniform_int_distribution<size_t> size_dist(64, 4096);
    std::thread producer([&] {
        for (int i = 0; i < count; ++i) {
            sizes[static_cast<size_t>(i)] = size_dist(state.rng);
            ptrs[static_cast<size_t>(i)] = s.vtable.allocate(sizes[static_cast<size_t>(i)]);
            touch_bytes(ptrs[static_cast<size_t>(i)], sizes[static_cast<size_t>(i)], 0x44);
        }
    });
    producer.join();
    for (int i = 0; i < count; ++i) metrics_alloc(m, sizes[static_cast<size_t>(i)]);
    std::thread consumer([&] {
        for (void* p : ptrs) s.vtable.deallocate(p);
    });
    consumer.join();
    for (int i = 0; i < count; ++i) metrics_free(m, sizes[static_cast<size_t>(i)], true);
}

static void cleanup_realtime_state(const StrategyDescriptor& s,
                                   GeneratedMetrics& m,
                                   RealtimePhaseState& state) {
    for (size_t i = 0; i < state.ptrs.size(); ++i) {
        if (state.ptrs[i]) {
            s.vtable.deallocate(state.ptrs[i]);
            metrics_free(m, state.sizes[i]);
            state.ptrs[i] = nullptr;
            state.sizes[i] = 0;
        }
    }
}

static void run_realtime_phase_chunk(const StrategyDescriptor& s,
                                     const BenchConfig& cfg,
                                     const GeneratedPhase& phase,
                                     int ops,
                                     GeneratedMetrics& m,
                                     RealtimePhaseState& state) {
    switch (phase.kind) {
        case GeneratedPhaseKind::SmallChurn:
            generated_small_churn_chunk(s, ops, cfg, m);
            break;
        case GeneratedPhaseKind::FragmentationDrift:
            generated_fragmentation_realtime_chunk(s, ops, phase.slots > 0 ? phase.slots : cfg.slots, m, state);
            break;
        case GeneratedPhaseKind::RemoteFree:
            generated_remote_realtime_chunk(s, ops, phase.threads > 0 ? phase.threads : cfg.threads, m, state);
            break;
        case GeneratedPhaseKind::LargeBurst:
            generated_large_burst_chunk(s, std::max(1, ops / 8), cfg, m);
            break;
        case GeneratedPhaseKind::PeakRelease:
            generated_peak_release_realtime_chunk(s, ops, phase.slots > 0 ? phase.slots : cfg.slots, m, state);
            break;
        case GeneratedPhaseKind::LatencyLoop:
            generated_latency_loop_chunk(s, ops, cfg, m);
            break;
    }
}

static void run_generated_realtime(const StrategyDescriptor& s,
                                   const BenchConfig& cfg,
                                   const std::vector<GeneratedPhase>& phases,
                                   GeneratedMetrics& m) {
    int phase_count = static_cast<int>(phases.size()) * std::max(1, cfg.phase_repeat);
    int phase_index = 0;
    for (int repeat = 0; repeat < std::max(1, cfg.phase_repeat); ++repeat) {
        for (size_t i = 0; i < phases.size(); ++i, ++phase_index) {
            const GeneratedPhase& phase = phases[i];
            int phase_ms = phase.duration_ms > 0 ? phase.duration_ms : cfg.phase_ms;
            int target_ops = phase.target_ops_per_sec > 0 ? phase.target_ops_per_sec : cfg.target_ops_per_sec;
            int tick_ms = std::max(10, std::min(100, phase_ms));
            int ops_per_tick = std::max(1, target_ops * tick_ms / 1000);
            RealtimePhaseState state(cfg.seed + static_cast<unsigned>(phase_index) * 977u);
            double phase_start = now_ms();
            while (true) {
                double elapsed = now_ms() - phase_start;
                if (elapsed >= phase_ms) break;
                double tick_start = now_ms();
                run_realtime_phase_chunk(s, cfg, phase, ops_per_tick, m, state);
                double after_ops = now_ms();
                double phase_elapsed = after_ops - phase_start;
                publish_generated(phase, phase_index, phase_count, m,
                                  phase_elapsed, static_cast<double>(phase_ms), true);
                double spent = now_ms() - tick_start;
                double remaining = static_cast<double>(phase_ms) - (now_ms() - phase_start);
                double sleep_ms = std::min(static_cast<double>(tick_ms) - spent, remaining);
                if (sleep_ms > 0.0) {
                    usleep(static_cast<useconds_t>(sleep_ms * 1000.0));
                }
            }
            cleanup_realtime_state(s, m, state);
            publish_generated(phase, phase_index, phase_count, m,
                              static_cast<double>(phase_ms), static_cast<double>(phase_ms), true);
        }
    }
}

static void run_tiny_cache_churn_phase(const StrategyDescriptor& s,
                                       const BenchConfig& cfg,
                                       GeneratedMetrics& m,
                                       int ops) {
    static constexpr size_t sizes[] = {16, 32, 64, 128};
    static constexpr int batch = 256;

    std::vector<PayloadRecord> ptrs(batch);
    uint64_t id = 1;
    int done = 0;

    while (done < ops) {
        int n = std::min(batch, ops - done);

        // Allocate a batch of tiny objects.
        for (int i = 0; i < n; ++i) {
            size_t size = sizes[(i + done) % 4];
            void* p = s.vtable.allocate(size);
            payload_write(p, size, static_cast<unsigned char>(id));
            ptrs[i] = {p, size, id++, static_cast<unsigned char>(id)};
            metrics_alloc(m, size);
        }

        // Free in reverse order to create strong local reuse / cache behavior.
        for (int i = n - 1; i >= 0; --i) {
            validation_check_record(cfg, m, ptrs[i]);
            s.vtable.deallocate(ptrs[i].ptr);
            metrics_free(m, ptrs[i].size, false);
            ptrs[i] = {};
        }

        done += n;
    }
}

static void run_remote_dominant_phase(const StrategyDescriptor& s,
                                      const BenchConfig& cfg,
                                      GeneratedMetrics& m,
                                      int ops,
                                      int threads,
                                      unsigned seed) {
    int worker_count = std::max(2, threads);
    int objects = std::max(worker_count * 256, ops / 2);

    std::vector<PayloadRecord> ptrs(static_cast<size_t>(objects));
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> size_dist(32, 2048);

    uint64_t id = 1;

    // Allocate all objects on the current thread.
    for (int i = 0; i < objects; ++i) {
        size_t size = static_cast<size_t>(size_dist(rng));
        void* p = s.vtable.allocate(size);
        payload_write(p, size, static_cast<unsigned char>(id));
        ptrs[static_cast<size_t>(i)] = {p, size, id++, static_cast<unsigned char>(id)};
        metrics_alloc(m, size);
    }

    // Free them from other threads. Metrics are updated after join to avoid races.
    std::vector<std::thread> workers;
    for (int t = 0; t < worker_count; ++t) {
        workers.emplace_back([&, t] {
            for (int i = t; i < objects; i += worker_count) {
                s.vtable.deallocate(ptrs[static_cast<size_t>(i)].ptr);
            }
        });
    }
    for (auto& th : workers) th.join();

    for (int i = 0; i < objects; ++i) {
        metrics_free(m, ptrs[static_cast<size_t>(i)].size, true);
        ptrs[static_cast<size_t>(i)] = {};
    }
}

static void run_medium_frag_strong_phase(const StrategyDescriptor& s,
                                         const BenchConfig& cfg,
                                         GeneratedMetrics& m,
                                         int ops,
                                         int slots,
                                         unsigned seed) {
    static constexpr size_t sizes[] = {
        1024, 1536, 2048, 3072, 4096,
        6144, 8192, 12288, 16384, 32768
    };

    int slot_count = std::max(128, slots);
    std::vector<PayloadRecord> ptrs(static_cast<size_t>(slot_count));

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> slot_dist(0, slot_count - 1);
    std::uniform_int_distribution<int> size_idx(0, static_cast<int>(std::size(sizes)) - 1);
    std::uniform_int_distribution<int> action_dist(0, 9);

    uint64_t id = 1;

    for (int i = 0; i < ops; ++i) {
        int idx = slot_dist(rng);
        PayloadRecord& rec = ptrs[static_cast<size_t>(idx)];
        int action = action_dist(rng);

        if (!rec.ptr) {
            size_t size = sizes[size_idx(rng)];
            void* p = s.vtable.allocate(size);
            payload_write(p, size, static_cast<unsigned char>(id));
            rec = {p, size, id++, static_cast<unsigned char>(id)};
            metrics_alloc(m, size);
            continue;
        }

        validation_check_record(cfg, m, rec);

        if (action < 4) {
            // Realloc to a different medium size to create size-class drift.
            size_t new_size = sizes[size_idx(rng)];
            void* next = s.vtable.reallocate(rec.ptr, new_size);
            if (next) {
                metrics_realloc(m, rec.size, new_size);
                rec.ptr = next;
                rec.size = new_size;
                rec.id = id++;
                rec.seed = static_cast<unsigned char>(id);
                payload_write(rec.ptr, rec.size, rec.seed);
            }
        } else {
            s.vtable.deallocate(rec.ptr);
            metrics_free(m, rec.size, false);
            rec = {};
        }
    }

    for (PayloadRecord& rec : ptrs) {
        if (rec.ptr) {
            validation_check_record(cfg, m, rec);
            s.vtable.deallocate(rec.ptr);
            metrics_free(m, rec.size, false);
            rec = {};
        }
    }
}

static void run_rss_decay_hold_phase(const StrategyDescriptor& s,
                                     const BenchConfig& cfg,
                                     GeneratedMetrics& m,
                                     int ops,
                                     int slots,
                                     unsigned seed) {
    int peak_objects = std::max(1024, slots * 4);
    std::vector<PayloadRecord> ptrs(static_cast<size_t>(peak_objects));

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> size_dist(4096, 32768);

    uint64_t id = 1;

    // Phase 1: build a memory peak.
    for (int i = 0; i < peak_objects; ++i) {
        size_t size = static_cast<size_t>(size_dist(rng));
        void* p = s.vtable.allocate(size);
        payload_write(p, size, static_cast<unsigned char>(id));
        ptrs[static_cast<size_t>(i)] = {p, size, id++, static_cast<unsigned char>(id)};
        metrics_alloc(m, size);
    }

    // Phase 2: release 90% objects, leaving mapped/live pressure.
    int release_count = peak_objects * 9 / 10;
    for (int i = 0; i < release_count; ++i) {
        PayloadRecord& rec = ptrs[static_cast<size_t>(i)];
        validation_check_record(cfg, m, rec);
        s.vtable.deallocate(rec.ptr);
        metrics_free(m, rec.size, false);
        rec = {};
    }

    // Phase 3: keep doing small activity while most memory should be reclaimed.
    static constexpr size_t small_sizes[] = {64, 128, 256, 512};
    for (int i = 0; i < ops; ++i) {
        size_t size = small_sizes[i % 4];
        void* p = s.vtable.allocate(size);
        payload_write(p, size, static_cast<unsigned char>(id));
        PayloadRecord rec{p, size, id++, static_cast<unsigned char>(id)};
        metrics_alloc(m, size);

        validation_check_record(cfg, m, rec);
        s.vtable.deallocate(p);
        metrics_free(m, size, false);
    }

    // Cleanup remaining 10%.
    for (PayloadRecord& rec : ptrs) {
        if (rec.ptr) {
            validation_check_record(cfg, m, rec);
            s.vtable.deallocate(rec.ptr);
            metrics_free(m, rec.size, false);
            rec = {};
        }
    }
}

static BenchResult generated_workload(const StrategyDescriptor& s,
                                      const BenchConfig& cfg) {
    std::string workload_name = cfg.workload_template;
    std::vector<GeneratedPhase> phases = cfg.workload_config.empty()
        ? generated_template(cfg.workload_template)
        : load_generated_config(cfg.workload_config, workload_name);
    if (phases.empty()) {
        phases = generated_template(cfg.workload_template);
    }
    int total_weight = 0;
    for (const auto& p : phases) total_weight += p.weight;
    total_weight = std::max(1, total_weight);
    GeneratedMetrics m;
    set_workload_identity(s.name, "generated_workload", workload_name.c_str());
    g_workload_start_ms = now_ms();
    int total_phase_count = static_cast<int>(phases.size()) * std::max(1, cfg.phase_repeat);
    update_workload_snapshot("starting", 0, total_phase_count, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0.0, cfg.workload_realtime ? static_cast<double>(cfg.phase_ms) : 0.0, true);
    double start = now_ms();
    if (cfg.workload_realtime) {
        run_generated_realtime(s, cfg, phases, m);
    } else {
        for (size_t i = 0; i < phases.size(); ++i) {
            const GeneratedPhase& phase = phases[i];
            int ops = std::max(1, cfg.iters * phase.weight / total_weight);
            int phase_slots = phase.slots > 0 ? phase.slots : cfg.slots;
            int phase_threads = phase.threads > 0 ? phase.threads : cfg.threads;
            publish_generated(phase, static_cast<int>(i), static_cast<int>(phases.size()), m, 0.0, 0.0, true);
            switch (phase.kind) {
                case GeneratedPhaseKind::SmallChurn:
                    generated_small_churn(s, ops, m, phase, static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::FragmentationDrift:
                    generated_fragmentation(s, ops, phase_slots, cfg.seed, m, phase,
                                            static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::RemoteFree:
                    generated_remote_free(s, ops, phase_threads, cfg.seed, m, phase,
                                          static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::LargeBurst:
                    generated_large_burst(s, std::max(1, ops / 8), m, phase,
                                          static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::PeakRelease:
                    generated_peak_release(s, ops, phase_slots, m, phase,
                                           static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::LatencyLoop:
                    generated_latency_loop(s, ops, m, phase, static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::TinyCacheChurn:
                    run_tiny_cache_churn_phase(
                        s,
                        cfg,
                        m,
                        ops
                    );
                    break;

                case GeneratedPhaseKind::RemoteDominant:
                    run_remote_dominant_phase(
                        s,
                        cfg,
                        m,
                        ops,
                        phase_threads,
                        cfg.seed + static_cast<unsigned>(i)
                    );
                    break;

                case GeneratedPhaseKind::MediumFragStrong:
                    run_medium_frag_strong_phase(
                        s,
                        cfg,
                        m,
                        ops,
                        phase_slots,
                        cfg.seed + static_cast<unsigned>(i)
                    );
                    break;

                case GeneratedPhaseKind::RssDecayHold:
                    run_rss_decay_hold_phase(
                        s,
                        cfg,
                        m,
                        ops,
                        phase_slots,
                        cfg.seed + static_cast<unsigned>(i)
                    );
                    break;
            }
        }
    }
    double end = now_ms();
    update_workload_snapshot("finished", total_phase_count, total_phase_count,
                             m.ops, m.allocs, m.frees, m.reallocs, m.remote_frees,
                             m.requested_bytes, m.live_objects, m.live_bytes, m.peak_live_bytes,
                             m.validation_checks, m.validation_errors,
                             0.0, 0.0, false);
    return {"generated_workload", end - start, static_cast<size_t>(m.ops), peak_rss_kb(),
            generated_extra_json(workload_name, m)};
}

static RepeatSummary summarize(const std::vector<double>& values) {
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    double sum = 0.0;
    for (double v : sorted) sum += v;
    double mean = sorted.empty() ? 0.0 : sum / static_cast<double>(sorted.size());
    double var = 0.0;
    for (double v : sorted) {
        double d = v - mean;
        var += d * d;
    }
    if (!sorted.empty()) var /= static_cast<double>(sorted.size());
    size_t p95_index = sorted.empty() ? 0 : std::min(sorted.size() - 1, sorted.size() * 95 / 100);
    return RepeatSummary{
        mean,
        sorted.empty() ? 0.0 : sorted[sorted.size() / 2],
        sorted.empty() ? 0.0 : sorted[p95_index],
        std::sqrt(var),
        sorted.empty() ? 0.0 : sorted.front(),
        sorted.empty() ? 0.0 : sorted.back(),
    };
}

static bool strategy_is_adaptive(const char* strategy) {
    return std::strcmp(strategy, "adaptive") == 0;
}

static void print_json(const char* strategy, const BenchResult& r) {
    double ops = r.ops / (r.ms / 1000.0);
    std::printf("{\"strategy\":\"%s\",\"benchmark\":\"%s\",\"ops_per_sec\":%.0f,\"ms\":%.3f,\"peak_rss_kb\":%zu",
                strategy, r.name.c_str(), ops, r.ms, r.peak_rss_kb);
    if (strategy_is_adaptive(strategy)) {
        my_ptmalloc::AdaptiveStatsSnapshot s = my_ptmalloc::adaptive_stats_snapshot();
        std::printf(",\"adaptive\":{\"current_mode\":\"%s\",\"active_mode\":\"%s\","
                    "\"previous_mode\":\"%s\",\"mode_switches\":%llu,"
                    "\"retired_mode_count\":%llu,\"mapped_bytes\":%lld,"
                    "\"live_bytes\":%lld,\"mapped_live_ratio\":%.3f,"
                    "\"remote_free_ratio\":%.3f,\"size_entropy\":%.3f,"
                    "\"large_bytes_ratio\":%.3f,\"fragmentation_estimate\":%.3f,"
                    "\"slow_path_ratio\":%.3f,\"double_free_count\":%llu,"
                    "\"invalid_free_count\":%llu,"
                    "\"empty_pages\":%llu,\"empty_spans\":%llu,\"released_pages\":%llu,"
                    "\"released_spans\":%llu,\"release_unmapped_bytes\":%llu,"
                    "\"mode_alloc_count\":[%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu],"
                    "\"mode_free_count\":[%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu],"
                    "\"mode_live_bytes\":[%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld],"
                    "\"mode_mapped_bytes\":[%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld],"
                    "\"storage_allocs\":[%llu,%llu,%llu],\"storage_frees\":[%llu,%llu,%llu],"
                    "\"pool_hits\":[%llu,%llu,%llu],\"pool_misses\":[%llu,%llu,%llu]}",
                    my_ptmalloc::adaptive_mode_name(s.current_mode),
                    my_ptmalloc::adaptive_mode_name(s.active_mode),
                    my_ptmalloc::adaptive_mode_name(s.previous_mode),
                    static_cast<unsigned long long>(s.mode_switches),
                    static_cast<unsigned long long>(s.retired_mode_count),
                    static_cast<long long>(s.mapped_bytes),
                    static_cast<long long>(s.live_bytes),
                    s.mapped_live_ratio,
                    s.remote_free_ratio,
                    s.size_entropy,
                    s.large_bytes_ratio,
                    s.fragmentation_estimate,
                    s.slow_path_ratio,
                    static_cast<unsigned long long>(s.double_free_count),
                    static_cast<unsigned long long>(s.invalid_free_count),
                    static_cast<unsigned long long>(s.empty_pages),
                    static_cast<unsigned long long>(s.empty_spans),
                    static_cast<unsigned long long>(s.released_pages),
                    static_cast<unsigned long long>(s.released_spans),
                    static_cast<unsigned long long>(s.release_unmapped_bytes),
                    static_cast<unsigned long long>(s.mode_alloc_count[0]),
                    static_cast<unsigned long long>(s.mode_alloc_count[1]),
                    static_cast<unsigned long long>(s.mode_alloc_count[2]),
                    static_cast<unsigned long long>(s.mode_alloc_count[3]),
                    static_cast<unsigned long long>(s.mode_alloc_count[4]),
                    static_cast<unsigned long long>(s.mode_alloc_count[5]),
                    static_cast<unsigned long long>(s.mode_alloc_count[6]),
                    static_cast<unsigned long long>(s.mode_alloc_count[7]),
                    static_cast<unsigned long long>(s.mode_free_count[0]),
                    static_cast<unsigned long long>(s.mode_free_count[1]),
                    static_cast<unsigned long long>(s.mode_free_count[2]),
                    static_cast<unsigned long long>(s.mode_free_count[3]),
                    static_cast<unsigned long long>(s.mode_free_count[4]),
                    static_cast<unsigned long long>(s.mode_free_count[5]),
                    static_cast<unsigned long long>(s.mode_free_count[6]),
                    static_cast<unsigned long long>(s.mode_free_count[7]),
                    static_cast<long long>(s.mode_live_bytes[0]),
                    static_cast<long long>(s.mode_live_bytes[1]),
                    static_cast<long long>(s.mode_live_bytes[2]),
                    static_cast<long long>(s.mode_live_bytes[3]),
                    static_cast<long long>(s.mode_live_bytes[4]),
                    static_cast<long long>(s.mode_live_bytes[5]),
                    static_cast<long long>(s.mode_live_bytes[6]),
                    static_cast<long long>(s.mode_live_bytes[7]),
                    static_cast<long long>(s.mode_mapped_bytes[0]),
                    static_cast<long long>(s.mode_mapped_bytes[1]),
                    static_cast<long long>(s.mode_mapped_bytes[2]),
                    static_cast<long long>(s.mode_mapped_bytes[3]),
                    static_cast<long long>(s.mode_mapped_bytes[4]),
                    static_cast<long long>(s.mode_mapped_bytes[5]),
                    static_cast<long long>(s.mode_mapped_bytes[6]),
                    static_cast<long long>(s.mode_mapped_bytes[7]),
                    static_cast<unsigned long long>(s.storage[0].alloc_count),
                    static_cast<unsigned long long>(s.storage[1].alloc_count),
                    static_cast<unsigned long long>(s.storage[2].alloc_count),
                    static_cast<unsigned long long>(s.storage[0].free_count),
                    static_cast<unsigned long long>(s.storage[1].free_count),
                    static_cast<unsigned long long>(s.storage[2].free_count),
                    static_cast<unsigned long long>(s.storage[0].pool_hits),
                    static_cast<unsigned long long>(s.storage[1].pool_hits),
                    static_cast<unsigned long long>(s.storage[2].pool_hits),
                    static_cast<unsigned long long>(s.storage[0].pool_misses),
                    static_cast<unsigned long long>(s.storage[1].pool_misses),
                    static_cast<unsigned long long>(s.storage[2].pool_misses));
    }
    if (!r.extra_json.empty()) {
        std::printf(",%s", r.extra_json.c_str());
    }
    std::printf("}\n");
}

static void print_usage(const char* argv0) {
    std::printf(
        "usage: %s [options]\n"
        "\n"
        "options:\n"
        "  --strategy NAME       hybrid, ptmalloc, tcmalloc_like, jemalloc_like,\n"
        "                        mimalloc_like, adaptive, libc, or plugin:path.so\n"
        "  --profile NAME        smoke, micro, stress, all (default: micro)\n"
        "  --bench NAME          add one benchmark; may be repeated\n"
        "                        same_size, same_size_64, same_size_256, batch, random,\n"
        "                        fragmentation, cross_thread_free, latency_sample,\n"
        "                        phase_changing, throughput_server, realtime_latency,\n"
        "                        memory_constrained, producer_consumer, large_streaming,\n"
        "                        debug_safety, generated_workload\n"
        "  --json                print one JSON object per result\n"
        "  --repeats N           repeat each benchmark and report aggregate stats\n"
        "  --iters N             iterations for same-size/fragmentation/latency tests\n"
        "  --random-iters N      iterations for random workload\n"
        "  --size N              size for same_size and latency_sample\n"
        "  --min-size N          random/cross-thread minimum allocation size\n"
        "  --max-size N          random/cross-thread maximum allocation size\n"
        "  --slots N             live pointer slots for random/fragmentation\n"
        "  --batch N             batch size for batch/cross-thread tests\n"
        "  --rounds N            batch rounds\n"
        "  --threads N           threads for cross_thread_free\n"
        "  --workload-template N adaptive_mix, throughput_churn, remote_queue,\n"
        "                        large_burst, rss_peak_release, fragmentation_drift,\n"
        "                        latency_loop\n"
        "  --workload-config P   JSON phase config for generated_workload\n"
        "  --workload-realtime   run generated_workload by wall-clock phase time\n"
        "  --phase-ms N          realtime generated_workload phase duration (default: 10000)\n"
        "  --target-ops-per-sec N realtime generated_workload throttle target (default: 50000)\n"
        "  --phase-repeat N      realtime generated_workload template repetitions\n"
        "  --payload-validation  sample deterministic payload checks in generated_workload\n"
        "  --payload-validation-rate N check every N generated operations (default: 64)\n"
        "  --telemetry-port N    serve lightweight local workload UI on 127.0.0.1:N\n"
        "  --telemetry-hold-ms N keep telemetry UI alive after benchmarks finish\n"
        "                        when --telemetry-port is enabled (default: 300000; 0 exits immediately)\n"
        "  --seed N              deterministic RNG seed\n"
        "  --help                show this help\n",
        argv0);
}

static bool parse_int_arg(const char* value, int& out) {
    char* end = nullptr;
    long v = std::strtol(value, &end, 10);
    if (!end || *end != '\0' || v <= 0) return false;
    out = static_cast<int>(v);
    return true;
}

static bool parse_nonnegative_int_arg(const char* value, int& out) {
    char* end = nullptr;
    long v = std::strtol(value, &end, 10);
    if (!end || *end != '\0' || v < 0) return false;
    out = static_cast<int>(v);
    return true;
}

static bool parse_size_arg(const char* value, size_t& out) {
    char* end = nullptr;
    unsigned long long v = std::strtoull(value, &end, 10);
    if (!end || *end != '\0' || v == 0) return false;
    out = static_cast<size_t>(v);
    return true;
}

static bool parse_args(int argc, char** argv, BenchConfig& cfg) {
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        auto need_value = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "%s needs a value\n", name);
                return nullptr;
            }
            return argv[++i];
        };

        if (std::strcmp(arg, "--strategy") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.strategy = v;
        } else if (std::strcmp(arg, "--profile") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.profile = v;
        } else if (std::strcmp(arg, "--bench") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.benches.emplace_back(v);
        } else if (std::strcmp(arg, "--json") == 0) {
            cfg.json = true;
        } else if (std::strcmp(arg, "--iters") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.iters)) return false;
        } else if (std::strcmp(arg, "--random-iters") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.random_iters)) return false;
        } else if (std::strcmp(arg, "--size") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_size_arg(v, cfg.size)) return false;
        } else if (std::strcmp(arg, "--min-size") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_size_arg(v, cfg.min_size)) return false;
        } else if (std::strcmp(arg, "--max-size") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_size_arg(v, cfg.max_size)) return false;
        } else if (std::strcmp(arg, "--slots") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.slots)) return false;
        } else if (std::strcmp(arg, "--batch") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.batch)) return false;
        } else if (std::strcmp(arg, "--rounds") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.rounds)) return false;
        } else if (std::strcmp(arg, "--threads") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.threads)) return false;
        } else if (std::strcmp(arg, "--workload-template") == 0 ||
                   std::strcmp(arg, "--template") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.workload_template = v;
        } else if (std::strcmp(arg, "--workload-config") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.workload_config = v;
        } else if (std::strcmp(arg, "--workload-realtime") == 0) {
            cfg.workload_realtime = true;
        } else if (std::strcmp(arg, "--phase-ms") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.phase_ms)) return false;
        } else if (std::strcmp(arg, "--target-ops-per-sec") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.target_ops_per_sec)) return false;
        } else if (std::strcmp(arg, "--phase-repeat") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.phase_repeat)) return false;
        } else if (std::strcmp(arg, "--payload-validation") == 0) {
            cfg.payload_validation = true;
        } else if (std::strcmp(arg, "--payload-validation-rate") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.payload_validation_rate)) return false;
        } else if (std::strcmp(arg, "--telemetry-port") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.telemetry_port)) return false;
        } else if (std::strcmp(arg, "--telemetry-hold-ms") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_nonnegative_int_arg(v, cfg.telemetry_hold_ms)) return false;
        } else if (std::strcmp(arg, "--repeats") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.repeats)) return false;
        } else if (std::strcmp(arg, "--seed") == 0) {
            const char* v = need_value(arg);
            size_t parsed = 0;
            if (!v || !parse_size_arg(v, parsed)) return false;
            cfg.seed = static_cast<unsigned>(parsed);
        } else if (std::strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            std::exit(0);
        } else {
            std::fprintf(stderr, "unknown option: %s\n", arg);
            return false;
        }
    }
    if (cfg.min_size > cfg.max_size) {
        std::fprintf(stderr, "--min-size must be <= --max-size\n");
        return false;
    }
    return true;
}

static std::vector<std::string> profile_benches(const std::string& profile) {
    if (profile == "smoke") return {"same_size_64", "random"};
    if (profile == "micro") return {"same_size_64", "same_size_256", "batch", "random"};
    if (profile == "stress") return {"random", "fragmentation", "cross_thread_free"};
    if (profile == "all") {
        return {"same_size_64", "same_size_256", "same_size", "batch", "random",
                "fragmentation", "cross_thread_free", "latency_sample", "phase_changing"};
    }
    std::fprintf(stderr, "unknown profile '%s'\n", profile.c_str());
    return {};
}

static bool run_one(const StrategyDescriptor& s,
                    const BenchConfig& cfg,
                    const std::string& name,
                    BenchResult& out) {
    if (name == "same_size") {
        out = same_size(s, cfg.size, cfg.iters);
        return true;
    }
    if (name == "same_size_64") {
        out = same_size(s, 64, cfg.iters);
        return true;
    }
    if (name == "same_size_256") {
        out = same_size(s, 256, cfg.iters);
        return true;
    }
    if (name == "batch") {
        out = batch_workload(s, cfg.batch, cfg.rounds);
        return true;
    }
    if (name == "random") {
        out = random_workload(s, cfg.random_iters, cfg.slots, cfg.min_size, cfg.max_size, cfg.seed);
        return true;
    }
    if (name == "fragmentation") {
        out = fragmentation_workload(s, cfg.iters, cfg.slots, cfg.min_size, cfg.max_size, cfg.seed);
        return true;
    }
    if (name == "cross_thread_free") {
        out = cross_thread_free_workload(s, cfg.threads, cfg.batch, cfg.min_size, cfg.max_size, cfg.seed);
        return true;
    }
    if (name == "latency_sample") {
        out = latency_sample_workload(s, cfg.size, cfg.iters, 100);
        return true;
    }
    if (name == "phase_changing") {
        out = phase_changing_workload(s, cfg.iters, cfg.slots, cfg.seed);
        return true;
    }
    if (name == "throughput_server") {
        out = batch_workload(s, cfg.batch * 2, cfg.rounds);
        out.name = "throughput_server";
        return true;
    }
    if (name == "realtime_latency") {
        out = latency_sample_workload(s, cfg.size, cfg.iters, 50);
        out.name = "realtime_latency";
        return true;
    }
    if (name == "memory_constrained") {
        out = phase_changing_workload(s, cfg.iters, cfg.slots, cfg.seed);
        out.name = "memory_constrained";
        return true;
    }
    if (name == "producer_consumer") {
        out = cross_thread_free_workload(s, cfg.threads, cfg.batch, cfg.min_size, cfg.max_size, cfg.seed);
        out.name = "producer_consumer";
        return true;
    }
    if (name == "large_streaming") {
        out = large_streaming_workload(s, std::max(1, cfg.iters / 8));
        return true;
    }
    if (name == "debug_safety") {
        out = debug_safety_workload(s, std::max(1, cfg.iters / 16), strategy_is_adaptive(s.name));
        return true;
    }
    if (name == "generated_workload") {
        out = generated_workload(s, cfg);
        return true;
    }
    std::fprintf(stderr, "unknown benchmark '%s'\n", name.c_str());
    return false;
}

int main(int argc, char** argv) {
    BenchConfig cfg;
    if (!parse_args(argc, argv, cfg)) {
        print_usage(argv[0]);
        return 2;
    }

    LoadedStrategy loaded;
    if (!load_strategy(cfg.strategy.c_str(), loaded)) return 2;
    if (loaded.desc.vtable.init) loaded.desc.vtable.init();

    TelemetryServer telemetry_server(telemetry_snapshot_json, WEB_VIEWER_DIR);
    if (cfg.telemetry_port > 0) {
        set_workload_identity(loaded.desc.name, "idle", cfg.workload_template.c_str());
        if (!telemetry_server.start(cfg.telemetry_port)) {
            std::fprintf(stderr, "failed to start telemetry server on 127.0.0.1:%d\n", cfg.telemetry_port);
            unload_strategy(loaded);
            return 2;
        }
        if (!cfg.json) {
            std::printf("telemetry UI: http://127.0.0.1:%d/\n", cfg.telemetry_port);
        }
    }

    std::vector<std::string> benches = cfg.benches.empty() ? profile_benches(cfg.profile) : cfg.benches;
    if (benches.empty()) {
        unload_strategy(loaded);
        return 2;
    }

    if (!cfg.json) {
        std::printf("=== strategy: %s ===\n", loaded.desc.name);
        std::printf("profile=%s iters=%d random_iters=%d size=%zu range=%zu..%zu slots=%d batch=%d rounds=%d threads=%d repeats=%d seed=%u\n",
                    cfg.profile.c_str(), cfg.iters, cfg.random_iters, cfg.size, cfg.min_size,
                    cfg.max_size, cfg.slots, cfg.batch, cfg.rounds, cfg.threads, cfg.repeats, cfg.seed);
    }

    for (const auto& bench : benches) {
        std::vector<double> ops_values;
        BenchResult last;
        for (int rep = 0; rep < cfg.repeats; ++rep) {
            BenchResult r;
            if (!run_one(loaded.desc, cfg, bench, r)) {
                unload_strategy(loaded);
                return 2;
            }
            last = r;
            ops_values.push_back(r.ops / (r.ms / 1000.0));
            if (cfg.json && cfg.repeats == 1) {
                print_json(loaded.desc.name, r);
            }
        }
        if (cfg.repeats > 1) {
            RepeatSummary summary = summarize(ops_values);
            if (cfg.json) {
                std::printf("{\"strategy\":\"%s\",\"benchmark\":\"%s\",\"repeats\":%d,"
                            "\"ops_per_sec_mean\":%.0f,\"ops_per_sec_median\":%.0f,"
                            "\"ops_per_sec_p95\":%.0f,\"ops_per_sec_stddev\":%.0f,"
                            "\"ops_per_sec_min\":%.0f,\"ops_per_sec_max\":%.0f,"
                            "\"last_ms\":%.3f,\"peak_rss_kb\":%zu",
                            loaded.desc.name, last.name.c_str(), cfg.repeats,
                            summary.mean, summary.median, summary.p95, summary.stddev,
                            summary.min, summary.max, last.ms, last.peak_rss_kb);
                if (strategy_is_adaptive(loaded.desc.name)) {
                    auto s = my_ptmalloc::adaptive_stats_snapshot();
                    std::printf(",\"adaptive\":{\"current_mode\":\"%s\",\"active_mode\":\"%s\","
                                "\"previous_mode\":\"%s\",\"mode_switches\":%llu,"
                                "\"retired_mode_count\":%llu,\"mapped_bytes\":%lld,"
                                "\"live_bytes\":%lld,\"mapped_live_ratio\":%.3f,"
                                "\"remote_free_ratio\":%.3f,\"size_entropy\":%.3f,"
                                "\"large_bytes_ratio\":%.3f,\"fragmentation_estimate\":%.3f,"
                                "\"slow_path_ratio\":%.3f,\"double_free_count\":%llu,"
                                "\"invalid_free_count\":%llu}",
                                my_ptmalloc::adaptive_mode_name(s.current_mode),
                                my_ptmalloc::adaptive_mode_name(s.active_mode),
                                my_ptmalloc::adaptive_mode_name(s.previous_mode),
                                static_cast<unsigned long long>(s.mode_switches),
                                static_cast<unsigned long long>(s.retired_mode_count),
                                static_cast<long long>(s.mapped_bytes),
                                static_cast<long long>(s.live_bytes),
                                s.mapped_live_ratio,
                                s.remote_free_ratio,
                                s.size_entropy,
                                s.large_bytes_ratio,
                                s.fragmentation_estimate,
                                s.slow_path_ratio,
                                static_cast<unsigned long long>(s.double_free_count),
                                static_cast<unsigned long long>(s.invalid_free_count));
                }
                std::printf("}\n");
            } else {
                std::printf("  %-16s mean=%10.0f median=%10.0f p95=%10.0f stddev=%8.0f min=%10.0f max=%10.0f peak=%zuKB\n",
                            last.name.c_str(), summary.mean, summary.median, summary.p95,
                            summary.stddev, summary.min, summary.max, last.peak_rss_kb);
            }
        } else if (!cfg.json) {
            double ops = ops_values.empty() ? 0.0 : ops_values.front();
            std::printf("  %-16s %10.0f ops/sec  %7.2f ms  peak=%zuKB\n",
                        last.name.c_str(), ops, last.ms, last.peak_rss_kb);
        }
    }

    if (cfg.telemetry_port > 0 && cfg.telemetry_hold_ms > 0 && !cfg.json) {
        std::printf("telemetry UI remains available for %.1f seconds; press Ctrl+C to stop earlier\n",
                    static_cast<double>(cfg.telemetry_hold_ms) / 1000.0);
        std::fflush(stdout);
        int remaining = cfg.telemetry_hold_ms;
        while (remaining > 0) {
            int chunk = std::min(remaining, 1000);
            usleep(static_cast<useconds_t>(chunk) * 1000);
            remaining -= chunk;
        }
    }

    unload_strategy(loaded);
    telemetry_server.stop();
    return 0;
}
