#include "strategy_loader.h"
#include "my_ptmalloc/adaptive_allocator.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <random>
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

static void touch_bytes(void* p, size_t size, unsigned char value) {
    if (!p) return;
    std::memset(p, value, std::min<size_t>(size, 64));
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
        "                        debug_safety\n"
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

    unload_strategy(loaded);
    return 0;
}
