// Stress test: multi-threaded random alloc/realloc/free with validation

#include "my_ptmalloc/my_malloc.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <vector>
#include <random>
#include <atomic>

using namespace my_ptmalloc;

static std::atomic<int> tests_passed{0};
static std::atomic<int> tests_failed{0};

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define PASS() do { tests_passed++; printf("OK\n"); } while(0)

// ─── Single-threaded stress: random alloc/free ───

static void test_random_alloc_free() {
    printf("  random_alloc_free ... ");

    constexpr int N = 500;
    void* ptrs[N] = {};
    size_t sizes[N] = {};
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(1, 2048);
    std::uniform_int_distribution<int> action_dist(0, 2);

    for (int i = 0; i < N * 4; ++i) {
        int idx = i % N;
        int action = action_dist(rng);

        if (action == 0 || ptrs[idx] == nullptr) {
            // Allocate
            size_t sz = size_dist(rng);
            void* p = my_malloc(sz);
            ASSERT(p != nullptr);
            // Use usable_size for memset to avoid writing past chunk boundary
            size_t usable = my_malloc_usable_size(p);
            memset(p, static_cast<int>(idx & 0xFF), usable);
            ptrs[idx] = p;
            sizes[idx] = sz;
        } else if (action == 1) {
            // Free
            my_free(ptrs[idx]);
            ptrs[idx] = nullptr;
            sizes[idx] = 0;
        } else {
            // Realloc
            size_t new_sz = size_dist(rng);
            void* p = my_realloc(ptrs[idx], new_sz);
            if (new_sz > 0) {
                ASSERT(p != nullptr);
            }
            ptrs[idx] = p;
            sizes[idx] = new_sz;
        }
    }

    // Free remaining
    for (int i = 0; i < N; ++i) {
        if (ptrs[i]) my_free(ptrs[i]);
    }

    PASS();
}

// ─── Single-threaded stress: alloc-free cycles ───

static void test_alloc_free_cycles() {
    printf("  alloc_free_cycles ... ");

    constexpr int CYCLES = 500;
    constexpr int BATCH = 50;

    for (int cycle = 0; cycle < CYCLES; ++cycle) {
        void* ptrs[BATCH];
        size_t sizes[BATCH];

        // Allocate batch with varying sizes
        for (int i = 0; i < BATCH; ++i) {
            sizes[i] = 16 + (cycle * BATCH + i) % 4000;
            ptrs[i] = my_malloc(sizes[i]);
            ASSERT(ptrs[i] != nullptr);
            size_t safe = sizes[i] < my_malloc_usable_size(ptrs[i])
                        ? sizes[i] : my_malloc_usable_size(ptrs[i]);
            memset(ptrs[i], 0xAB, safe);
        }

        // Verify all patterns
        for (int i = 0; i < BATCH; ++i) {
            unsigned char* p = static_cast<unsigned char*>(ptrs[i]);
            size_t safe = sizes[i] < my_malloc_usable_size(ptrs[i])
                        ? sizes[i] : my_malloc_usable_size(ptrs[i]);
            for (size_t j = 0; j < safe; ++j) {
                ASSERT(p[j] == 0xAB);
            }
        }

        // Free batch
        for (int i = 0; i < BATCH; ++i) {
            my_free(ptrs[i]);
        }
    }

    PASS();
}

// ─── Multi-threaded stress ───

struct ThreadArg {
    int id;
    int iterations;
    int seed;
    std::atomic<int>* error_count;
};

static void* thread_worker(void* arg) {
    ThreadArg* ta = static_cast<ThreadArg*>(arg);
    std::mt19937 rng(ta->seed);
    std::uniform_int_distribution<size_t> size_dist(1, 2048);
    std::uniform_int_distribution<int> action_dist(0, 3);

    constexpr int SLOTS = 64;
    void* ptrs[SLOTS] = {};
    size_t sizes[SLOTS] = {};

    for (int i = 0; i < ta->iterations; ++i) {
        int idx = rng() % SLOTS;
        int action = action_dist(rng);

        if (ptrs[idx] == nullptr || action <= 1) {
            // Allocate
            size_t sz = size_dist(rng);
            void* p = my_malloc(sz);
            if (!p) {
                ta->error_count->fetch_add(1);
                continue;
            }
            // Write pattern based on thread id and slot
            size_t usable = my_malloc_usable_size(p);
            memset(p, static_cast<int>((ta->id * 17 + idx) & 0xFF), usable);
            ptrs[idx] = p;
            sizes[idx] = sz;
        } else if (action == 2 && ptrs[idx]) {
            // Realloc
            size_t new_sz = size_dist(rng);
            void* p = my_realloc(ptrs[idx], new_sz);
            if (new_sz > 0 && !p) {
                ta->error_count->fetch_add(1);
                continue;
            }
            ptrs[idx] = p;
            sizes[idx] = new_sz;
        } else if (action == 3 && ptrs[idx]) {
            // Free
            my_free(ptrs[idx]);
            ptrs[idx] = nullptr;
            sizes[idx] = 0;
        }
    }

    // Free remaining
    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) {
            my_free(ptrs[i]);
            ptrs[i] = nullptr;
        }
    }

    return nullptr;
}

static void test_multithreaded_stress() {
    printf("  multithreaded_stress ... ");

    constexpr int NUM_THREADS = 4;
    constexpr int ITERATIONS = 2000;

    pthread_t threads[NUM_THREADS];
    ThreadArg args[NUM_THREADS];
    std::atomic<int> error_count{0};

    for (int i = 0; i < NUM_THREADS; ++i) {
        args[i] = {i, ITERATIONS, i * 1000 + 42, &error_count};
        pthread_create(&threads[i], nullptr, thread_worker, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    ASSERT(error_count.load() == 0);
    PASS();
}

// ─── Multi-threaded: same-size alloc/free ───

static void* same_size_worker(void* arg) {
    ThreadArg* ta = static_cast<ThreadArg*>(arg);

    for (int i = 0; i < ta->iterations; ++i) {
        void* p = my_malloc(128);
        if (!p) {
            ta->error_count->fetch_add(1);
            continue;
        }
        memset(p, ta->id & 0xFF, 128);
        my_free(p);
    }

    return nullptr;
}

static void test_multithreaded_same_size() {
    printf("  multithreaded_same_size ... ");

    constexpr int NUM_THREADS = 4;
    constexpr int ITERATIONS = 5000;

    pthread_t threads[NUM_THREADS];
    ThreadArg args[NUM_THREADS];
    std::atomic<int> error_count{0};

    for (int i = 0; i < NUM_THREADS; ++i) {
        args[i] = {i, ITERATIONS, i * 1000 + 42, &error_count};
        pthread_create(&threads[i], nullptr, same_size_worker, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    ASSERT(error_count.load() == 0);
    PASS();
}

// ─── Edge cases ───

static void test_edge_cases() {
    printf("  edge_cases ... ");

    // Free nullptr
    my_free(nullptr);

    // Realloc nullptr (should act like malloc)
    void* p = my_realloc(nullptr, 100);
    ASSERT(p != nullptr);
    my_free(p);

    // Realloc to size 0 (should act like free)
    p = my_malloc(100);
    ASSERT(p != nullptr);
    void* q = my_realloc(p, 0);
    ASSERT(q == nullptr);

    // Malloc(0)
    p = my_malloc(0);
    ASSERT(p != nullptr);
    my_free(p);

    // Calloc overflow
    q = my_calloc(SIZE_MAX / 2, 3);
    ASSERT(q == nullptr);

    PASS();
}

// ─── Usable size ───

static void test_usable_size() {
    printf("  usable_size ... ");

    for (size_t sz = 1; sz <= 1024; sz = sz < 64 ? sz + 1 : sz * 2) {
        void* p = my_malloc(sz);
        ASSERT(p != nullptr);
        size_t usable = my_malloc_usable_size(p);
        ASSERT(usable >= sz);
        // Write to full usable area — should not crash
        memset(p, 0xCC, usable);
        my_free(p);
    }

    // nullptr
    ASSERT(my_malloc_usable_size(nullptr) == 0);

    PASS();
}

int main() {
    printf("=== stress tests ===\n");

    test_random_alloc_free();
    test_alloc_free_cycles();
    test_multithreaded_stress();
    test_multithreaded_same_size();
    test_edge_cases();
    test_usable_size();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed.load(), tests_failed.load());
    return tests_failed.load() > 0 ? 1 : 0;
}
