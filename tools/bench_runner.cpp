#include "strategy_loader.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
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
    unsigned seed = 12345;
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

static void print_json(const char* strategy, const BenchResult& r) {
    double ops = r.ops / (r.ms / 1000.0);
    std::printf("{\"strategy\":\"%s\",\"benchmark\":\"%s\",\"ops_per_sec\":%.0f,\"ms\":%.3f,\"peak_rss_kb\":%zu}\n",
                strategy, r.name.c_str(), ops, r.ms, r.peak_rss_kb);
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
        "                        fragmentation, cross_thread_free, latency_sample\n"
        "  --json                print one JSON object per result\n"
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
                "fragmentation", "cross_thread_free", "latency_sample"};
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
        std::printf("profile=%s iters=%d random_iters=%d size=%zu range=%zu..%zu slots=%d batch=%d rounds=%d threads=%d seed=%u\n",
                    cfg.profile.c_str(), cfg.iters, cfg.random_iters, cfg.size, cfg.min_size,
                    cfg.max_size, cfg.slots, cfg.batch, cfg.rounds, cfg.threads, cfg.seed);
    }

    for (const auto& bench : benches) {
        BenchResult r;
        if (!run_one(loaded.desc, cfg, bench, r)) {
            unload_strategy(loaded);
            return 2;
        }
        if (cfg.json) {
            print_json(loaded.desc.name, r);
        } else {
            double ops = r.ops / (r.ms / 1000.0);
            std::printf("  %-16s %10.0f ops/sec  %7.2f ms  peak=%zuKB\n",
                        r.name.c_str(), ops, r.ms, r.peak_rss_kb);
        }
    }

    unload_strategy(loaded);
    return 0;
}
