// Basic allocation tests for my_ptmalloc

#include "my_ptmalloc/my_malloc.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

using namespace my_ptmalloc;

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL at line %d: %s\n", __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define PASS() do { tests_passed++; printf("OK\n"); } while(0)

// ─── Test: basic malloc and free ───

static void test_malloc_free_basic() {
    printf("  malloc_free_basic ... ");
    void* p = my_malloc(100);
    ASSERT(p != nullptr);
    memset(p, 0xAB, 100);
    unsigned char* bytes = static_cast<unsigned char*>(p);
    for (int i = 0; i < 100; i++) ASSERT(bytes[i] == 0xAB);
    my_free(p);
    PASS();
}

// ─── Test: multiple allocations ───

static void test_malloc_multiple() {
    printf("  malloc_multiple ... ");
    void* ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = my_malloc(64 * (i + 1));
        ASSERT(ptrs[i] != nullptr);
        memset(ptrs[i], i, 64 * (i + 1));
    }
    for (int i = 0; i < 10; i++) {
        for (int j = i + 1; j < 10; j++) {
            ASSERT(ptrs[i] != ptrs[j]);
        }
    }
    for (int i = 0; i < 10; i++) {
        unsigned char* bytes = static_cast<unsigned char*>(ptrs[i]);
        for (size_t j = 0; j < 64 * (i + 1); j++) {
            ASSERT(bytes[j] == static_cast<unsigned char>(i));
        }
    }
    for (int i = 0; i < 10; i++) my_free(ptrs[i]);
    PASS();
}

// ─── Test: calloc ───

static void test_calloc_basic() {
    printf("  calloc_basic ... ");
    void* p = my_calloc(10, 100);
    ASSERT(p != nullptr);
    unsigned char* bytes = static_cast<unsigned char*>(p);
    for (int i = 0; i < 1000; i++) ASSERT(bytes[i] == 0);
    my_free(p);
    PASS();
}

// ─── Test: realloc ───

static void test_realloc_basic() {
    printf("  realloc_basic ... ");
    void* p = my_malloc(50);
    ASSERT(p != nullptr);
    memset(p, 0x42, 50);
    void* p2 = my_realloc(p, 200);
    ASSERT(p2 != nullptr);
    unsigned char* bytes = static_cast<unsigned char*>(p2);
    for (int i = 0; i < 50; i++) ASSERT(bytes[i] == 0x42);
    my_free(p2);
    PASS();
}

// ─── Test: realloc(NULL, size) == malloc(size) ───

static void test_realloc_null() {
    printf("  realloc_null ... ");
    void* p = my_realloc(nullptr, 100);
    ASSERT(p != nullptr);
    memset(p, 0x55, 100);
    my_free(p);
    PASS();
}

// ─── Test: zero-size malloc ───

static void test_malloc_zero() {
    printf("  malloc_zero ... ");
    void* p = my_malloc(0);
    ASSERT(p != nullptr);
    my_free(p);
    PASS();
}

// ─── Test: large allocation ───

static void test_malloc_large() {
    printf("  malloc_large ... ");
    void* p = my_malloc(1024 * 1024);
    ASSERT(p != nullptr);
    memset(p, 0xCC, 1024 * 1024);
    my_free(p);
    PASS();
}

// ─── Test: small allocation ───

static void test_malloc_small() {
    printf("  malloc_small ... ");
    void* p = my_malloc(1);
    ASSERT(p != nullptr);
    my_free(p);
    PASS();
}

// ─── Test: alignment ───

static void test_memalign_basic() {
    printf("  memalign_basic ... ");
    void* p = my_memalign(64, 100);
    ASSERT(p != nullptr);
    ASSERT((reinterpret_cast<uintptr_t>(p) & 63) == 0);
    memset(p, 0xDD, 100);
    my_free(p);
    PASS();
}

// ─── Test: alloc and free cycle ───

static void test_alloc_free_cycle() {
    printf("  alloc_free_cycle ... ");
    for (int i = 0; i < 100; i++) {
        void* p = my_malloc(128);
        ASSERT(p != nullptr);
        memset(p, i & 0xFF, 128);
        my_free(p);
    }
    PASS();
}

// ─── Test: data integrity ───

static void test_data_integrity() {
    printf("  data_integrity ... ");
    void* a = my_malloc(256);
    void* b = my_malloc(256);
    void* c = my_malloc(256);
    ASSERT(a && b && c);

    memset(a, 0xAA, 256);
    memset(b, 0xBB, 256);
    memset(c, 0xCC, 256);

    my_free(b);

    unsigned char* pa = static_cast<unsigned char*>(a);
    unsigned char* pc = static_cast<unsigned char*>(c);
    for (int i = 0; i < 256; i++) {
        ASSERT(pa[i] == 0xAA);
        ASSERT(pc[i] == 0xCC);
    }

    my_free(a);
    my_free(c);
    PASS();
}

// ─── Main ───

int main() {
    printf("=== my_ptmalloc basic tests ===\n");

    test_malloc_free_basic();
    test_malloc_multiple();
    test_calloc_basic();
    test_realloc_basic();
    test_realloc_null();
    test_malloc_zero();
    test_malloc_large();
    test_malloc_small();
    test_memalign_basic();
    test_alloc_free_cycle();
    test_data_integrity();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
