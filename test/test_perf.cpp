// Performance benchmark for my_ptmalloc

#include "my_ptmalloc/my_malloc.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <random>
#include <pthread.h>

using namespace std::chrono;
using namespace my_ptmalloc;

// ─── Benchmark: random alloc/free single-threaded ───

static double bench_random_alloc_free(int iterations) {
    constexpr int SLOTS = 1024;
    void* ptrs[SLOTS] = {};
    std::mt19937 rng(12345);
    std::uniform_int_distribution<size_t> size_dist(16, 4096);
    std::uniform_int_distribution<int> action_dist(0, 3);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        int idx = rng() % SLOTS;
        int action = action_dist(rng);

        if (ptrs[idx] == nullptr || action <= 1) {
            size_t sz = size_dist(rng);
            void* p = my_malloc(sz);
            if (p) memset(p, 0xAB, sz);
            ptrs[idx] = p;
        } else if (action == 2 && ptrs[idx]) {
            size_t new_sz = size_dist(rng);
            void* p = my_realloc(ptrs[idx], new_sz);
            ptrs[idx] = p;
        } else if (action == 3 && ptrs[idx]) {
            my_free(ptrs[idx]);
            ptrs[idx] = nullptr;
        }
    }

    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) my_free(ptrs[i]);
    }

    auto end = high_resolution_clock::now();
    return duration<double, std::milli>(end - start).count();
}

// ─── Benchmark: same-size alloc/free (tcache/fastbin hot path) ───

static double bench_same_size(int iterations) {
    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        void* p = my_malloc(64);
        memset(p, 0xCD, 64);
        my_free(p);
    }

    auto end = high_resolution_clock::now();
    return duration<double, std::milli>(end - start).count();
}

// ─── Benchmark: batch alloc then free ───

static double bench_batch(int batch_size, int batches) {
    auto start = high_resolution_clock::now();

    for (int b = 0; b < batches; ++b) {
        void* ptrs[batch_size];
        for (int i = 0; i < batch_size; ++i) {
            size_t sz = 128 + i % 512;
            ptrs[i] = my_malloc(sz);
            memset(ptrs[i], 0xEF, sz);
        }
        for (int i = 0; i < batch_size; ++i) {
            my_free(ptrs[i]);
        }
    }

    auto end = high_resolution_clock::now();
    return duration<double, std::milli>(end - start).count();
}

// ─── Benchmark: large allocations ───

static double bench_large_alloc(int iterations) {
    constexpr int SLOTS = 64;
    void* ptrs[SLOTS] = {};
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(8192, 65536);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        int idx = i % SLOTS;
        if (ptrs[idx]) my_free(ptrs[idx]);
        size_t sz = size_dist(rng);
        ptrs[idx] = my_malloc(sz);
        if (ptrs[idx]) memset(ptrs[idx], 0x55, sz);
    }

    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) my_free(ptrs[i]);
    }

    auto end = high_resolution_clock::now();
    return duration<double, std::milli>(end - start).count();
}

// ─── Benchmark: multi-threaded alloc/free ───

struct ThreadArg {
    int iterations;
    int seed;
};

static void* bench_thread_worker(void* arg) {
    ThreadArg* ta = static_cast<ThreadArg*>(arg);
    constexpr int SLOTS = 128;
    void* ptrs[SLOTS] = {};
    std::mt19937 rng(ta->seed);
    std::uniform_int_distribution<size_t> size_dist(16, 2048);
    std::uniform_int_distribution<int> action_dist(0, 3);

    for (int i = 0; i < ta->iterations; ++i) {
        int idx = rng() % SLOTS;
        int action = action_dist(rng);

        if (ptrs[idx] == nullptr || action <= 1) {
            size_t sz = size_dist(rng);
            ptrs[idx] = my_malloc(sz);
            if (ptrs[idx]) memset(ptrs[idx], ta->seed & 0xFF, sz);
        } else if (action == 2 && ptrs[idx]) {
            ptrs[idx] = my_realloc(ptrs[idx], size_dist(rng));
        } else if (action == 3 && ptrs[idx]) {
            my_free(ptrs[idx]);
            ptrs[idx] = nullptr;
        }
    }

    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) my_free(ptrs[i]);
    }
    return nullptr;
}

static double bench_multithreaded(int num_threads, int iterations) {
    pthread_t threads[num_threads];
    ThreadArg args[num_threads];

    auto start = high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        args[i] = {iterations, i * 1000 + 42};
        pthread_create(&threads[i], nullptr, bench_thread_worker, &args[i]);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    auto end = high_resolution_clock::now();
    return duration<double, std::milli>(end - start).count();
}

// ─── Main ───

int main() {
    printf("=== my_ptmalloc performance benchmarks ===\n\n");

    // Warm up
    for (int i = 0; i < 10000; ++i) {
        void* p = my_malloc(64);
        my_free(p);
    }

    {
        int n = 200000;
        double ms = bench_random_alloc_free(n);
        printf("  random_alloc_free  (%dk): %10.0f ops/sec  (%6.1f ms)\n",
               n/1000, n / (ms / 1000.0), ms);
    }

    {
        int n = 1000000;
        double ms = bench_same_size(n);
        printf("  same_size 64B    (%dk): %10.0f ops/sec  (%6.1f ms)\n",
               n/1000, n / (ms / 1000.0), ms);
    }

    {
        int batch = 256, batches = 2000;
        double ms = bench_batch(batch, batches);
        int total = batch * batches;
        printf("  batch 256x2000   (%dk): %10.0f ops/sec  (%6.1f ms)\n",
               total/1000, total / (ms / 1000.0), ms);
    }

    {
        int n = 50000;
        double ms = bench_large_alloc(n);
        printf("  large_alloc 8-64k (%dk): %10.0f ops/sec  (%6.1f ms)\n",
               n/1000, n / (ms / 1000.0), ms);
    }

    {
        int threads = 4, n = 100000;
        double ms = bench_multithreaded(threads, n);
        int total = threads * n;
        printf("  mt %dx%d       (%dk): %10.0f ops/sec  (%6.1f ms)\n",
               threads, n/1000, total/1000, total / (ms / 1000.0), ms);
    }

    printf("\n=== done ===\n");
    return 0;
}
