// ptmalloc correctness tests.

#include "../ptmalloc.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <random>
#include <thread>
#include <vector>
#include <atomic>

using namespace my_ptmalloc::ptmalloc;

static std::atomic<int> passed{0};
static std::atomic<int> failed{0};

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        failed++; return; \
    } \
} while(0)

#define PASS(name) do { \
    std::printf("  %s ... OK\n", name); \
    passed++; \
} while(0)

// ─── Basic tests ───

static void test_malloc_free() {
    void* p = pt_malloc(100);
    CHECK(p != nullptr);
    std::memset(p, 0xAB, 100);
    pt_free(p);
    PASS("malloc_free");
}

static void test_malloc_zero() {
    void* p = pt_malloc(0);
    CHECK(p != nullptr);
    pt_free(p);
    PASS("malloc_zero");
}

static void test_malloc_small() {
    for (int i = 0; i < 100; i++) {
        void* p = pt_malloc(1);
        CHECK(p != nullptr);
        pt_free(p);
    }
    PASS("malloc_small");
}

static void test_malloc_large() {
    void* p = pt_malloc(2 * 1024 * 1024);  // 2MB — direct mmap
    CHECK(p != nullptr);
    std::memset(p, 0xCC, 2 * 1024 * 1024);
    pt_free(p);
    PASS("malloc_large");
}

static void test_calloc() {
    void* p = pt_calloc(100, 10);
    CHECK(p != nullptr);
    auto* bytes = static_cast<unsigned char*>(p);
    for (int i = 0; i < 1000; i++) CHECK(bytes[i] == 0);
    pt_free(p);
    PASS("calloc");
}

static void test_calloc_overflow() {
    void* p = pt_calloc(SIZE_MAX / 2, 3);
    CHECK(p == nullptr);
    PASS("calloc_overflow");
}

static void test_realloc_grow() {
    void* p = pt_malloc(50);
    CHECK(p != nullptr);
    std::memset(p, 0x42, 50);
    void* q = pt_realloc(p, 200);
    CHECK(q != nullptr);
    auto* bytes = static_cast<unsigned char*>(q);
    for (int i = 0; i < 50; i++) CHECK(bytes[i] == 0x42);
    pt_free(q);
    PASS("realloc_grow");
}

static void test_realloc_null() {
    void* p = pt_realloc(nullptr, 100);
    CHECK(p != nullptr);
    pt_free(p);
    PASS("realloc_null");
}

static void test_realloc_zero() {
    void* p = pt_malloc(100);
    CHECK(p != nullptr);
    void* q = pt_realloc(p, 0);
    CHECK(q == nullptr);
    PASS("realloc_zero");
}

static void test_memalign() {
    for (size_t align : {64, 128, 256, 512, 4096}) {
        void* p = pt_memalign(align, 100);
        CHECK(p != nullptr);
        CHECK((reinterpret_cast<uintptr_t>(p) & (align - 1)) == 0);
        std::memset(p, 0xDD, 100);
        pt_free(p);
    }
    PASS("memalign");
}

static void test_usable_size() {
    for (size_t sz = 1; sz <= 1024; sz = sz < 64 ? sz + 1 : sz * 2) {
        void* p = pt_malloc(sz);
        CHECK(p != nullptr);
        size_t usable = pt_malloc_usable_size(p);
        CHECK(usable >= sz);
        std::memset(p, 0xCC, usable);
        pt_free(p);
    }
    CHECK(pt_malloc_usable_size(nullptr) == 0);
    PASS("usable_size");
}

static void test_data_integrity() {
    void* a = pt_malloc(256);
    void* b = pt_malloc(256);
    void* c = pt_malloc(256);
    CHECK(a && b && c);

    std::memset(a, 0xAA, 256);
    std::memset(b, 0xBB, 256);
    std::memset(c, 0xCC, 256);

    pt_free(b);  // free middle

    auto* pa = static_cast<unsigned char*>(a);
    auto* pc = static_cast<unsigned char*>(c);
    for (int i = 0; i < 256; i++) {
        CHECK(pa[i] == 0xAA);
        CHECK(pc[i] == 0xCC);
    }

    pt_free(a);
    pt_free(c);
    PASS("data_integrity");
}

static void test_free_null() {
    pt_free(nullptr);  // should be safe
    PASS("free_null");
}

// ─── Multi-size stress ───

static void test_many_sizes() {
    std::vector<void*> ptrs;
    for (size_t sz = 16; sz <= 65536; sz <<= 1) {
        void* p = pt_malloc(sz);
        CHECK(p != nullptr);
        std::memset(p, 0xEF, sz);
        ptrs.push_back(p);
    }
    for (void* p : ptrs) pt_free(p);
    PASS("many_sizes");
}

// ─── Random fuzzing ───

static void test_random_fuzz() {
    constexpr int SLOTS = 256;
    constexpr int OPS   = 10000;
    void* ptrs[SLOTS] = {};
    size_t sizes[SLOTS] = {};

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(1, 4096);
    std::uniform_int_distribution<int> idx_dist(0, SLOTS - 1);
    std::uniform_int_distribution<int> action_dist(0, 3);

    for (int i = 0; i < OPS; i++) {
        int idx = idx_dist(rng);
        int action = (ptrs[idx] == nullptr) ? 0 : action_dist(rng);

        switch (action) {
        case 0:  // alloc
            if (ptrs[idx]) pt_free(ptrs[idx]);
            sizes[idx] = size_dist(rng);
            ptrs[idx] = pt_malloc(sizes[idx]);
            CHECK(ptrs[idx] != nullptr);
            std::memset(ptrs[idx], idx & 0xFF, sizes[idx]);
            break;
        case 1:  // free
            pt_free(ptrs[idx]);
            ptrs[idx] = nullptr;
            break;
        case 2: {  // realloc
            size_t new_sz = size_dist(rng);
            void* p = pt_realloc(ptrs[idx], new_sz);
            CHECK(p != nullptr);
            ptrs[idx] = p;
            sizes[idx] = new_sz;
            break;
        }
        case 3:  // usable_size check
            CHECK(pt_malloc_usable_size(ptrs[idx]) >= sizes[idx]);
            break;
        }
    }

    // Free remaining
    for (int i = 0; i < SLOTS; i++) {
        if (ptrs[i]) pt_free(ptrs[i]);
    }
    PASS("random_fuzz");
}

// ─── Multi-threaded ───

static void test_multithreaded() {
    constexpr int N_THREADS = 4;
    constexpr int OPS_PER_THREAD = 5000;

    std::atomic<int> errors{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < N_THREADS; t++) {
        threads.emplace_back([t, &errors]() {
            std::mt19937 rng(t * 100 + 42);
            std::uniform_int_distribution<size_t> size_dist(1, 2048);
            std::uniform_int_distribution<int> action_dist(0, 2);

            constexpr int SLOTS = 32;
            void* ptrs[SLOTS] = {};
            size_t sizes[SLOTS] = {};

            for (int i = 0; i < OPS_PER_THREAD; i++) {
                int idx = std::uniform_int_distribution<int>(0, SLOTS - 1)(rng);
                int action = (ptrs[idx] == nullptr) ? 0 : action_dist(rng);

                if (action == 0) {
                    if (ptrs[idx]) pt_free(ptrs[idx]);
                    sizes[idx] = size_dist(rng);
                    ptrs[idx] = pt_malloc(sizes[idx]);
                    if (!ptrs[idx]) errors++;
                    else std::memset(ptrs[idx], (t * 17 + idx) & 0xFF, sizes[idx]);
                } else if (action == 1) {
                    pt_free(ptrs[idx]);
                    ptrs[idx] = nullptr;
                } else {
                    size_t new_sz = size_dist(rng);
                    void* p = pt_realloc(ptrs[idx], new_sz);
                    if (!p) errors++;
                    else { ptrs[idx] = p; sizes[idx] = new_sz; }
                }
            }

            for (int i = 0; i < SLOTS; i++) {
                if (ptrs[i]) pt_free(ptrs[i]);
            }
        });
    }

    for (auto& th : threads) th.join();
    CHECK(errors.load() == 0);
    PASS("multithreaded");
}

// ─── Cross-thread free ───

static void test_cross_thread_free() {
    constexpr int N_THREADS = 4;
    constexpr int N_ALLOCS  = 1000;

    std::vector<void*> all_ptrs;
    for (int i = 0; i < N_ALLOCS; i++) {
        void* p = pt_malloc(64);
        CHECK(p != nullptr);
        std::memset(p, 0x99, 64);
        all_ptrs.push_back(p);
    }

    std::atomic<int> errors{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < N_THREADS; t++) {
        threads.emplace_back([t, &all_ptrs, &errors]() {
            for (size_t i = t; i < all_ptrs.size(); i += N_THREADS) {
                pt_free(all_ptrs[i]);
            }
        });
    }

    for (auto& th : threads) th.join();
    CHECK(errors.load() == 0);
    PASS("cross_thread_free");
}

// ─── Main ───

int main() {
    std::printf("=== ptmalloc tests ===\n");

    pt_malloc_init();

    test_malloc_free();
    test_malloc_zero();
    test_malloc_small();
    test_malloc_large();
    test_calloc();
    test_calloc_overflow();
    test_realloc_grow();
    test_realloc_null();
    test_realloc_zero();
    test_memalign();
    test_usable_size();
    test_data_integrity();
    test_free_null();
    test_many_sizes();
    test_random_fuzz();
    test_multithreaded();
    test_cross_thread_free();

    std::printf("\n=== %d passed, %d failed ===\n",
               passed.load(), failed.load());
    return failed.load() > 0 ? 1 : 0;
}
