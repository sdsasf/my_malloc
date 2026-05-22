// mimalloc correctness tests.

#include "../mimalloc.h"

#include <cstdio>
#include <cstring>
#include <random>
#include <thread>
#include <vector>
#include <atomic>

using namespace my_ptmalloc::mimalloc;

static std::atomic<int> passed{0};
static std::atomic<int> failed{0};

#define CHECK(cond) do { \
    if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failed++; return; } \
} while(0)
#define PASS(name) do { std::printf("  %s ... OK\n", name); passed++; } while(0)

static void test_malloc_free() {
    void* p = mi_malloc(100);
    CHECK(p != nullptr);
    std::memset(p, 0xAB, 100);
    mi_free(p);
    PASS("malloc_free");
}

static void test_malloc_zero() {
    void* p = mi_malloc(0);
    CHECK(p != nullptr);
    mi_free(p);
    PASS("malloc_zero");
}

static void test_various_sizes() {
    for (size_t sz : {1, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128,
                       255, 256, 511, 512, 1023, 1024, 2047, 2048,
                       4095, 4096, 8191, 8192, 16384, 65536, 131072}) {
        void* p = mi_malloc(sz);
        CHECK(p != nullptr);
        std::memset(p, 0xCD, sz < 256 ? sz : 256);
        mi_free(p);
    }
    PASS("various_sizes");
}

static void test_calloc() {
    void* p = mi_calloc(100, 16);
    CHECK(p != nullptr);
    auto* bytes = static_cast<unsigned char*>(p);
    for (int i = 0; i < 1600; i++) CHECK(bytes[i] == 0);
    mi_free(p);
    PASS("calloc");
}

static void test_realloc() {
    void* p = mi_malloc(50);
    CHECK(p != nullptr);
    std::memset(p, 0x42, 50);
    void* q = mi_realloc(p, 200);
    CHECK(q != nullptr);
    auto* bytes = static_cast<unsigned char*>(q);
    for (int i = 0; i < 50; i++) CHECK(bytes[i] == 0x42);
    mi_free(q);
    PASS("realloc");
}

static void test_realloc_null() {
    void* p = mi_realloc(nullptr, 100);
    CHECK(p != nullptr);
    mi_free(p);
    PASS("realloc_null");
}

static void test_realloc_zero() {
    void* p = mi_malloc(100);
    CHECK(p != nullptr);
    void* q = mi_realloc(p, 0);
    CHECK(q == nullptr);
    PASS("realloc_zero");
}

static void test_memalign() {
    for (size_t align : {64, 128, 256, 512, 4096}) {
        void* p = mi_memalign(align, 100);
        CHECK(p != nullptr);
        CHECK((reinterpret_cast<uintptr_t>(p) & (align - 1)) == 0);
        std::memset(p, 0xDD, 100);
        mi_free(p);
    }
    PASS("memalign");
}

static void test_usable_size() {
    for (size_t sz = 1; sz <= 4096; sz <<= 1) {
        void* p = mi_malloc(sz);
        CHECK(p != nullptr);
        size_t usable = mi_usable_size(p);
        CHECK(usable >= sz);
        mi_free(p);
    }
    PASS("usable_size");
}

static void test_random_fuzz() {
    constexpr int SLOTS = 256;
    constexpr int OPS = 5000;
    void* ptrs[SLOTS] = {};
    size_t sizes[SLOTS] = {};

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(1, 8192);
    std::uniform_int_distribution<int> action_dist(0, 2);
    std::uniform_int_distribution<int> slot_dist(0, SLOTS - 1);

    for (int i = 0; i < OPS; i++) {
        int idx = slot_dist(rng);
        int action = ptrs[idx] ? action_dist(rng) : 0;

        switch (action) {
        case 0:
            if (ptrs[idx]) mi_free(ptrs[idx]);
            sizes[idx] = size_dist(rng);
            ptrs[idx] = mi_malloc(sizes[idx]);
            CHECK(ptrs[idx] != nullptr);
            std::memset(ptrs[idx], idx & 0xFF, sizes[idx] < 256 ? sizes[idx] : 256);
            break;
        case 1:
            mi_free(ptrs[idx]);
            ptrs[idx] = nullptr;
            break;
        case 2: {
            size_t ns = size_dist(rng);
            void* np = mi_realloc(ptrs[idx], ns);
            CHECK(np != nullptr);
            ptrs[idx] = np;
            sizes[idx] = ns;
            break;
        }
        }
    }

    for (int i = 0; i < SLOTS; i++) {
        if (ptrs[i]) mi_free(ptrs[i]);
    }
    PASS("random_fuzz");
}

static void test_cross_thread_free() {
    constexpr int N_ALLOCS = 2000;
    std::vector<void*> ptrs;

    for (int i = 0; i < N_ALLOCS; i++) {
        void* p = mi_malloc(64);
        CHECK(p != nullptr);
        std::memset(p, 0x99, 64);
        ptrs.push_back(p);
    }

    // Free from other threads
    constexpr int N_THREADS = 4;
    std::vector<std::thread> threads;
    std::atomic<int> errors{0};

    for (int t = 0; t < N_THREADS; t++) {
        threads.emplace_back([t, &ptrs, &errors]() {
            for (size_t i = t; i < ptrs.size(); i += N_THREADS) {
                mi_free(ptrs[i]);
            }
        });
    }

    for (auto& th : threads) th.join();
    CHECK(errors.load() == 0);
    PASS("cross_thread_free");
}

static void test_multithreaded() {
    constexpr int N_THREADS = 4;
    constexpr int OPS = 3000;
    std::vector<std::thread> threads;
    std::atomic<int> errors{0};

    for (int t = 0; t < N_THREADS; t++) {
        threads.emplace_back([t, &errors]() {
            std::mt19937 rng(t * 200 + 42);
            std::uniform_int_distribution<size_t> sz(1, 2048);
            constexpr int S = 32;
            void* p[S] = {};
            size_t ss[S] = {};

            for (int i = 0; i < OPS; i++) {
                int idx = std::uniform_int_distribution<int>(0, S-1)(rng);
                if (p[idx]) { mi_free(p[idx]); p[idx] = nullptr; }
                ss[idx] = sz(rng);
                p[idx] = mi_malloc(ss[idx]);
                if (!p[idx]) errors++;
                else std::memset(p[idx], t & 0xFF, ss[idx] < 128 ? ss[idx] : 128);
            }
            for (int i = 0; i < S; i++) {
                if (p[i]) mi_free(p[i]);
            }
        });
    }

    for (auto& th : threads) th.join();
    CHECK(errors.load() == 0);
    PASS("multithreaded");
}

int main() {
    std::printf("=== mimalloc tests ===\n");
    mi_init();

    test_malloc_free();
    test_malloc_zero();
    test_various_sizes();
    test_calloc();
    test_realloc();
    test_realloc_null();
    test_realloc_zero();
    test_memalign();
    test_usable_size();
    test_random_fuzz();
    test_cross_thread_free();
    test_multithreaded();

    std::printf("\n=== %d passed, %d failed ===\n", passed.load(), failed.load());
    return failed.load() > 0 ? 1 : 0;
}
