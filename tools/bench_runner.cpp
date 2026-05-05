#include "strategy_loader.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <random>
#include <string>
#include <sys/resource.h>
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

static BenchResult same_size(const StrategyDescriptor& s, size_t size, int iters) {
    volatile unsigned char acc = 0;
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        void* p = s.vtable.allocate(size);
        std::memset(p, 0xCD, size);
        acc += *static_cast<unsigned char*>(p);
        s.vtable.deallocate(p);
    }
    double end = now_ms();
    char name[64];
    std::snprintf(name, sizeof(name), "same_size_%zu", size);
    return {name, end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult random_workload(const StrategyDescriptor& s, int iters) {
    constexpr int slots = 4096;
    std::vector<void*> ptrs(slots, nullptr);
    std::mt19937 rng(12345);
    std::uniform_int_distribution<size_t> size_dist(16, 8192);
    std::uniform_int_distribution<int> action_dist(0, 4);
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        int idx = static_cast<int>(rng() % slots);
        int action = action_dist(rng);
        if (!ptrs[idx] || action <= 2) {
            void* p = s.vtable.allocate(size_dist(rng));
            if (p) std::memset(p, 0xAB, 16);
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

static BenchResult batch_workload(const StrategyDescriptor& s, int batch, int rounds) {
    double start = now_ms();
    for (int r = 0; r < rounds; ++r) {
        std::vector<void*> ptrs(batch);
        for (int i = 0; i < batch; ++i) {
            size_t size = 128 + i % 512;
            ptrs[i] = s.vtable.allocate(size);
            if (ptrs[i]) std::memset(ptrs[i], 0xEF, size);
        }
        for (void* p : ptrs) s.vtable.deallocate(p);
    }
    double end = now_ms();
    return {"batch", end - start, static_cast<size_t>(batch) * rounds, peak_rss_kb()};
}

static void print_json(const char* strategy, const BenchResult& r) {
    double ops = r.ops / (r.ms / 1000.0);
    std::printf("{\"strategy\":\"%s\",\"benchmark\":\"%s\",\"ops_per_sec\":%.0f,\"ms\":%.3f,\"peak_rss_kb\":%zu}\n",
                strategy, r.name.c_str(), ops, r.ms, r.peak_rss_kb);
}

int main(int argc, char** argv) {
    const char* strategy = "hybrid";
    bool json = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--strategy") == 0 && i + 1 < argc) strategy = argv[++i];
        else if (std::strcmp(argv[i], "--json") == 0) json = true;
    }

    LoadedStrategy loaded;
    if (!load_strategy(strategy, loaded)) return 2;
    if (loaded.desc.vtable.init) loaded.desc.vtable.init();

    BenchResult results[] = {
        same_size(loaded.desc, 64, 1000000),
        same_size(loaded.desc, 256, 1000000),
        batch_workload(loaded.desc, 512, 1000),
        random_workload(loaded.desc, 200000),
    };

    if (!json) std::printf("=== strategy: %s ===\n", loaded.desc.name);
    for (const auto& r : results) {
        if (json) {
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
