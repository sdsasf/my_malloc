// Benchmark: my_ptmalloc vs system malloc
// Compile two versions:
//   bench_my    - linked with my_ptmalloc_static (uses my_malloc/my_free)
//   bench_sys   - no special linking (uses system malloc/free)
// Both run identical workloads and report throughput + memory footprint.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <pthread.h>
#include <unistd.h>
#include <sys/resource.h>
#include <time.h>

#ifdef USE_MY_MALLOC
#include "my_ptmalloc/my_malloc.h"
using namespace my_ptmalloc;
#define ALLOC   my_malloc
#define FREE    my_free
#define REALLOC my_realloc
#define CALLOC  my_calloc
#define LABEL   "my_ptmalloc"
#else
#define ALLOC   malloc
#define FREE    free
#define REALLOC realloc
#define CALLOC  calloc
#define LABEL   "glibc"
#endif

// ─── High-resolution timer ───

static double now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

// ─── RSS measurement ───

static size_t get_rss_kb() {
    FILE* f = fopen("/proc/self/statm", "r");
    if (!f) return 0;
    long pages = 0;
    (void)fscanf(f, "%*s %ld", &pages);
    fclose(f);
    return pages * (sysconf(_SC_PAGESIZE) / 1024);
}

static long get_peak_rss_kb() {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_maxrss;  // KB on Linux
}

// ─── Workloads ───

struct BenchResult {
    double ms;
    size_t peak_rss_kb;
    size_t live_rss_kb;
    size_t ops;
};

static BenchResult bench_random(int iterations) {
    constexpr int SLOTS = 4096;
    void* ptrs[SLOTS] = {};
    size_t sizes[SLOTS] = {};

    std::mt19937 rng(12345);
    std::uniform_int_distribution<size_t> size_dist(16, 8192);
    std::uniform_int_distribution<int> action_dist(0, 4);

    double start = now_ms();

    for (int i = 0; i < iterations; ++i) {
        int idx = rng() % SLOTS;
        int action = action_dist(rng);

        if (ptrs[idx] == nullptr || action <= 2) {
            size_t sz = size_dist(rng);
            void* p = ALLOC(sz);
            if (p) memset(p, 0xAB, sz);
            if (ptrs[idx]) FREE(ptrs[idx]);
            ptrs[idx] = p;
            sizes[idx] = sz;
        } else if (action == 3 && ptrs[idx]) {
            size_t new_sz = size_dist(rng);
            ptrs[idx] = REALLOC(ptrs[idx], new_sz);
            sizes[idx] = new_sz;
        } else if (action == 4 && ptrs[idx]) {
            FREE(ptrs[idx]);
            ptrs[idx] = nullptr;
            sizes[idx] = 0;
        }
    }

    size_t live_rss = get_rss_kb();

    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) FREE(ptrs[i]);
    }

    double end = now_ms();
    return {end - start, (size_t)get_peak_rss_kb(), live_rss, (size_t)iterations};
}

static BenchResult bench_samesize(int iterations, size_t sz) {
    volatile unsigned char acc = 0;
    double start = now_ms();
    for (int i = 0; i < iterations; ++i) {
        void* p = ALLOC(sz);
        memset(p, 0xCD, sz);
        acc += *static_cast<unsigned char*>(p);
        FREE(p);
    }
    double end = now_ms();
    return {end - start, (size_t)get_peak_rss_kb(), get_rss_kb(), (size_t)iterations};
}

static BenchResult bench_batch(int batch_size, int batches) {
    size_t total = 0;
    volatile unsigned char acc = 0;
    double start = now_ms();
    for (int b = 0; b < batches; ++b) {
        void* ptrs[batch_size];
        for (int i = 0; i < batch_size; ++i) {
            size_t sz = 128 + i % 512;
            ptrs[i] = ALLOC(sz);
            memset(ptrs[i], 0xEF, sz);
            acc += *static_cast<unsigned char*>(ptrs[i]);
        }
        for (int i = 0; i < batch_size; ++i) {
            FREE(ptrs[i]);
        }
        total += batch_size;
    }
    double end = now_ms();
    return {end - start, (size_t)get_peak_rss_kb(), get_rss_kb(), total};
}

static BenchResult bench_large(int iterations) {
    constexpr int SLOTS = 128;
    void* ptrs[SLOTS] = {};
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(8192, 131072);

    double start = now_ms();
    for (int i = 0; i < iterations; ++i) {
        int idx = i % SLOTS;
        if (ptrs[idx]) FREE(ptrs[idx]);
        size_t sz = size_dist(rng);
        ptrs[idx] = ALLOC(sz);
        if (ptrs[idx]) memset(ptrs[idx], 0x55, sz);
    }
    size_t live_rss = get_rss_kb();
    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) FREE(ptrs[i]);
    }
    double end = now_ms();
    return {end - start, (size_t)get_peak_rss_kb(), live_rss, (size_t)iterations};
}

static BenchResult bench_fragmentation() {
    constexpr int N = 10000;
    void* ptrs[N];

    for (int i = 0; i < N; ++i) {
        ptrs[i] = ALLOC(48);
        memset(ptrs[i], 0xAA, 48);
    }

    for (int i = 0; i < N; i += 2) {
        FREE(ptrs[i]);
        ptrs[i] = nullptr;
    }

    size_t rss_after_frag = get_rss_kb();

    double start = now_ms();
    for (int i = 0; i < N; i += 2) {
        ptrs[i] = ALLOC(96);
        if (ptrs[i]) memset(ptrs[i], 0xBB, 96);
    }
    double end = now_ms();

    size_t rss_after_realloc = get_rss_kb();

    for (int i = 0; i < N; ++i) {
        if (ptrs[i]) FREE(ptrs[i]);
    }

    printf("    frag: RSS after holes=%zuKB, after realloc=%zuKB\n",
           rss_after_frag, rss_after_realloc);
    return {end - start, (size_t)get_peak_rss_kb(), rss_after_frag, (size_t)(N / 2)};
}

struct ThreadArg {
    int iterations;
    int seed;
};

static void* mt_worker(void* arg) {
    ThreadArg* ta = (ThreadArg*)arg;
    constexpr int SLOTS = 256;
    void* ptrs[SLOTS] = {};
    std::mt19937 rng(ta->seed);
    std::uniform_int_distribution<size_t> size_dist(16, 2048);
    std::uniform_int_distribution<int> action_dist(0, 3);

    for (int i = 0; i < ta->iterations; ++i) {
        int idx = rng() % SLOTS;
        int action = action_dist(rng);
        if (ptrs[idx] == nullptr || action <= 1) {
            size_t sz = size_dist(rng);
            ptrs[idx] = ALLOC(sz);
            if (ptrs[idx]) memset(ptrs[idx], ta->seed & 0xFF, sz);
        } else if (action == 2 && ptrs[idx]) {
            ptrs[idx] = REALLOC(ptrs[idx], size_dist(rng));
        } else if (action == 3 && ptrs[idx]) {
            FREE(ptrs[idx]);
            ptrs[idx] = nullptr;
        }
    }
    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) FREE(ptrs[i]);
    }
    return nullptr;
}

static BenchResult bench_multithreaded(int num_threads, int iterations) {
    pthread_t threads[num_threads];
    ThreadArg args[num_threads];

    double start = now_ms();
    for (int i = 0; i < num_threads; ++i) {
        args[i] = {iterations, i * 1000 + 42};
        pthread_create(&threads[i], nullptr, mt_worker, &args[i]);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    double end = now_ms();

    size_t total = num_threads * iterations;
    return {end - start, (size_t)get_peak_rss_kb(), get_rss_kb(), total};
}

// ─── Helpers ───

static void print_result(const char* name, const BenchResult& r) {
    double ops_per_sec = r.ops / (r.ms / 1000.0);
    printf("  %-26s %10.0f ops/sec  %7.1f ms  peak=%zuKB\n",
           name, ops_per_sec, r.ms, r.peak_rss_kb);
}

// ─── Main ───

int main() {
    printf("=== %s benchmark ===\n\n", LABEL);

    // Warm up
    for (int i = 0; i < 5000; ++i) {
        void* p = ALLOC(64);
        FREE(p);
    }

    print_result("random_alloc_free (200k)", bench_random(200000));
    print_result("same_size 32B (10M)", bench_samesize(10000000, 32));
    print_result("same_size 64B (10M)", bench_samesize(10000000, 64));
    print_result("same_size 256B (10M)", bench_samesize(10000000, 256));
    print_result("batch 512x1000 (512k)", bench_batch(512, 1000));
    print_result("large 8-128k (30k)", bench_large(30000));
    print_result("fragmentation (5k reallocs)", bench_fragmentation());
    print_result("mt 4x50k (200k)", bench_multithreaded(4, 50000));

    printf("\n  Peak RSS: %zu KB\n", (size_t)get_peak_rss_kb());

    printf("\n=== done ===\n");
    return 0;
}
