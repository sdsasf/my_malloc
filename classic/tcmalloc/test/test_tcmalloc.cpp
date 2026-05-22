// tcmalloc correctness tests.

#include "../tcmalloc.h"

#include <cstdio>
#include <cstring>
#include <random>
#include <thread>
#include <vector>
#include <atomic>

using namespace my_ptmalloc::tcmalloc;

static std::atomic<int> passed{0};
static std::atomic<int> failed{0};

#define CHECK(cond) do { \
    if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failed++; return; } \
} while(0)
#define PASS(name) do { std::printf("  %s ... OK\n", name); passed++; } while(0)

static void test_malloc_free() {
    void* p = tc_malloc(100);
    CHECK(p != nullptr);
    std::memset(p, 0xAB, 100);
    tc_free(p);
    PASS("malloc_free");
}

static void test_various_sizes() {
    for (size_t sz : {1, 8, 16, 24, 32, 64, 128, 256, 512, 1024, 2048,
                       4096, 8192, 16384, 65536, 131072, 262144, 524288}) {
        void* p = tc_malloc(sz);
        CHECK(p != nullptr);
        std::memset(p, 0xCD, sz < 256 ? sz : 256);
        tc_free(p);
    }
    PASS("various_sizes");
}

static void test_calloc() {
    void* p = tc_calloc(100, 16);
    CHECK(p != nullptr);
    auto* b = static_cast<unsigned char*>(p);
    for (int i = 0; i < 1600; i++) CHECK(b[i] == 0);
    tc_free(p);
    PASS("calloc");
}

static void test_realloc() {
    void* p = tc_malloc(50);
    CHECK(p != nullptr);
    std::memset(p, 0x42, 50);
    void* q = tc_realloc(p, 200);
    CHECK(q != nullptr);
    for (int i = 0; i < 50; i++) CHECK(static_cast<unsigned char*>(q)[i] == 0x42);
    tc_free(q);
    PASS("realloc");
}

static void test_usable_size() {
    for (size_t sz = 1; sz <= 4096; sz <<= 1) {
        void* p = tc_malloc(sz);
        CHECK(p != nullptr);
        CHECK(tc_usable_size(p) >= sz);
        tc_free(p);
    }
    PASS("usable_size");
}

static void test_random_fuzz() {
    constexpr int S = 256;
    void* p[S] = {};
    size_t ss[S] = {};
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> sd(1, 8192);
    std::uniform_int_distribution<int> ad(0, 2), id(0, S-1);

    for (int i = 0; i < 5000; i++) {
        int idx = id(rng);
        int a = p[idx] ? ad(rng) : 0;
        if (a == 0) {
            if (p[idx]) tc_free(p[idx]);
            ss[idx] = sd(rng);
            p[idx] = tc_malloc(ss[idx]);
            CHECK(p[idx] != nullptr);
            std::memset(p[idx], idx & 0xFF, ss[idx] < 256 ? ss[idx] : 256);
        } else if (a == 1) { tc_free(p[idx]); p[idx] = nullptr; }
        else { p[idx] = tc_realloc(p[idx], sd(rng)); CHECK(p[idx] != nullptr); }
    }
    for (int i = 0; i < S; i++) if (p[i]) tc_free(p[i]);
    PASS("random_fuzz");
}

static void test_multithreaded() {
    constexpr int N = 4, O = 3000;
    std::vector<std::thread> th;
    std::atomic<int> err{0};

    for (int t = 0; t < N; t++) {
        th.emplace_back([t, &err]() {
            std::mt19937 rng(t * 200 + 42);
            std::uniform_int_distribution<size_t> sd(1, 2048);
            constexpr int S = 32;
            void* p[S] = {};
            for (int i = 0; i < O; i++) {
                int idx = std::uniform_int_distribution<int>(0, S-1)(rng);
                if (p[idx]) { tc_free(p[idx]); p[idx] = nullptr; }
                p[idx] = tc_malloc(sd(rng));
                if (!p[idx]) err++;
                else std::memset(p[idx], t & 0xFF, 64);
            }
            for (int i = 0; i < S; i++) if (p[i]) tc_free(p[i]);
        });
    }
    for (auto& t : th) t.join();
    CHECK(err.load() == 0);
    PASS("multithreaded");
}

int main() {
    std::printf("=== tcmalloc tests ===\n");
    tc_init_allocator();

    test_malloc_free();
    test_various_sizes();
    test_calloc();
    test_realloc();
    test_usable_size();
    test_random_fuzz();
    test_multithreaded();

    std::printf("\n=== %d passed, %d failed ===\n", passed.load(), failed.load());
    return failed.load() > 0 ? 1 : 0;
}
