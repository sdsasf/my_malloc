// Tcache subsystem tests

#include "my_ptmalloc/tcache.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/types.h"
#include "my_ptmalloc/chunk.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>

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

// Helper: allocate a raw chunk of given chunk size via mmap
static Chunk* make_chunk(size_t chunk_size) {
    void* p = mmap(nullptr, chunk_size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return nullptr;
    Chunk* c = reinterpret_cast<Chunk*>(p);
    c->prev_size = 0;
    c->set_head(ChunkSize{chunk_size}, ChunkFlag::PREV_INUSE);
    return c;
}

static void free_chunk(Chunk* c) {
    munmap(c, c->chunk_size().value);
}

// ─── Test: safe-linking protect/reveal roundtrip ───

static void test_safe_linking() {
    printf("  safe_linking ... ");

    // protect_ptr(pos, ptr) and reveal_ptr should roundtrip
    TcacheEntry* pos = reinterpret_cast<TcacheEntry*>(0x7f0000001000);
    TcacheEntry* ptr = reinterpret_cast<TcacheEntry*>(0x7f0000002000);

    TcacheEntry* protected_ptr = TcachePerthread::protect_ptr(pos, ptr);
    TcacheEntry* revealed = TcachePerthread::reveal_ptr(pos, protected_ptr);

    ASSERT(revealed == ptr);

    // Different position should give different protected value
    TcacheEntry* pos2 = reinterpret_cast<TcacheEntry*>(0x7f0000003000);
    TcacheEntry* protected_ptr2 = TcachePerthread::protect_ptr(pos2, ptr);
    ASSERT(protected_ptr != protected_ptr2);  // Different position => different mangled value

    // Null pointer roundtrip
    TcacheEntry* null_protected = TcachePerthread::protect_ptr(pos, nullptr);
    TcacheEntry* null_revealed = TcachePerthread::reveal_ptr(pos, null_protected);
    ASSERT(null_revealed == nullptr);

    PASS();
}

// ─── Test: basic alloc and free ───

static void test_basic_alloc_free() {
    printf("  basic_alloc_free ... ");

    // Ensure tcache is initialized
    if (!tcache) tcache_init();
    ASSERT(tcache != nullptr);

    // Create a chunk of size 64 (tidx = (64-32)/16 = 2)
    size_t cs = 64;
    TcacheIdx tidx = csize2tidx(ChunkSize{cs});
    ASSERT(tidx.value == 2);

    Chunk* c = make_chunk(cs);
    ASSERT(c != nullptr);

    // Free to tcache
    bool freed = tcache->free(tidx, c);
    ASSERT(freed);
    ASSERT(tcache->counts_[tidx.value] == 1);

    // Alloc from tcache
    void* p = tcache->alloc(tidx);
    ASSERT(p != nullptr);
    ASSERT(p == c->user_data());
    ASSERT(tcache->counts_[tidx.value] == 0);

    // Second alloc should return nullptr (empty)
    void* p2 = tcache->alloc(tidx);
    ASSERT(p2 == nullptr);

    free_chunk(c);
    PASS();
}

// ─── Test: LIFO order ───

static void test_lifo_order() {
    printf("  lifo_order ... ");

    if (!tcache) tcache_init();

    size_t cs = 64;
    TcacheIdx tidx = csize2tidx(ChunkSize{cs});

    Chunk* c1 = make_chunk(cs);
    Chunk* c2 = make_chunk(cs);
    Chunk* c3 = make_chunk(cs);
    ASSERT(c1 && c2 && c3);

    // Free in order: c1, c2, c3
    ASSERT(tcache->free(tidx, c1));
    ASSERT(tcache->free(tidx, c2));
    ASSERT(tcache->free(tidx, c3));
    ASSERT(tcache->counts_[tidx.value] == 3);

    // Alloc should return in LIFO order: c3, c2, c1
    void* p1 = tcache->alloc(tidx);
    ASSERT(p1 == c3->user_data());

    void* p2 = tcache->alloc(tidx);
    ASSERT(p2 == c2->user_data());

    void* p3 = tcache->alloc(tidx);
    ASSERT(p3 == c1->user_data());

    free_chunk(c1);
    free_chunk(c2);
    free_chunk(c3);
    PASS();
}

// ─── Test: fill count limit ───

static void test_fill_count_limit() {
    printf("  fill_count_limit ... ");

    if (!tcache) tcache_init();

    size_t cs = 64;
    TcacheIdx tidx = csize2tidx(ChunkSize{cs});

    // Fill to TCACHE_FILL_COUNT (16)
    Chunk* chunks[TCACHE_FILL_COUNT + 1];
    for (size_t i = 0; i <= TCACHE_FILL_COUNT; ++i) {
        chunks[i] = make_chunk(cs);
        ASSERT(chunks[i] != nullptr);
    }

    for (size_t i = 0; i < TCACHE_FILL_COUNT; ++i) {
        ASSERT(tcache->free(tidx, chunks[i]));
    }
    ASSERT(tcache->counts_[tidx.value] == TCACHE_FILL_COUNT);

    // 17th free should fail (bin full)
    ASSERT(!tcache->free(tidx, chunks[TCACHE_FILL_COUNT]));
    ASSERT(tcache->counts_[tidx.value] == TCACHE_FILL_COUNT);

    // Drain all
    for (size_t i = 0; i < TCACHE_FILL_COUNT; ++i) {
        void* p = tcache->alloc(tidx);
        ASSERT(p != nullptr);
    }
    ASSERT(tcache->counts_[tidx.value] == 0);

    for (size_t i = 0; i <= TCACHE_FILL_COUNT; ++i) {
        free_chunk(chunks[i]);
    }
    PASS();
}

// ─── Test: multiple bins ───

static void test_multiple_bins() {
    printf("  multiple_bins ... ");

    if (!tcache) tcache_init();

    // Use different chunk sizes for different bins
    size_t sizes[] = {32, 48, 64, 96, 128, 256};
    size_t nsizes = sizeof(sizes) / sizeof(sizes[0]);

    Chunk* chunks[6];
    for (size_t i = 0; i < nsizes; ++i) {
        chunks[i] = make_chunk(sizes[i]);
        ASSERT(chunks[i] != nullptr);

        TcacheIdx tidx = csize2tidx(ChunkSize{sizes[i]});
        ASSERT(tidx.value < TCACHE_MAX_BINS);
        ASSERT(tcache->free(tidx, chunks[i]));
    }

    // Verify counts
    for (size_t i = 0; i < nsizes; ++i) {
        TcacheIdx tidx = csize2tidx(ChunkSize{sizes[i]});
        ASSERT(tcache->counts_[tidx.value] == 1);
    }

    // Alloc from each bin
    for (size_t i = 0; i < nsizes; ++i) {
        TcacheIdx tidx = csize2tidx(ChunkSize{sizes[i]});
        void* p = tcache->alloc(tidx);
        ASSERT(p == chunks[i]->user_data());
    }

    for (size_t i = 0; i < nsizes; ++i) {
        free_chunk(chunks[i]);
    }
    PASS();
}

// ─── Test: double-free detection ───

static void test_double_free_detection() {
    printf("  double_free_detection ... ");

    if (!tcache) tcache_init();

    size_t cs = 64;
    TcacheIdx tidx = csize2tidx(ChunkSize{cs});

    Chunk* c = make_chunk(cs);
    ASSERT(c != nullptr);

    // Free once
    ASSERT(tcache->free(tidx, c));

    // Check key is set (double-free detection marker)
    TcacheEntry* e = reinterpret_cast<TcacheEntry*>(c->user_data());
    ASSERT(tcache->is_double_free(e));

    // Alloc to clear
    void* p = tcache->alloc(tidx);
    ASSERT(p == c->user_data());

    // After alloc, the key should still match (it's in the chunk memory)
    // but the entry is no longer in tcache, so is_double_free on a
    // freshly allocated chunk is expected to still match (the key wasn't cleared).
    // In real glibc, the key is checked BEFORE inserting into tcache.

    free_chunk(c);
    PASS();
}

// ─── Test: chunk header preserved across tcache cycle ───

static void test_chunk_header_preserved() {
    printf("  chunk_header_preserved ... ");

    if (!tcache) tcache_init();

    size_t cs = 128;
    TcacheIdx tidx = csize2tidx(ChunkSize{cs});

    Chunk* c = make_chunk(cs);
    ASSERT(c != nullptr);

    // Verify header before free
    ASSERT(c->chunk_size().value == cs);
    ASSERT(c->prev_inuse());

    // Free to tcache (writes TcacheEntry at user_data, NOT at chunk header)
    ASSERT(tcache->free(tidx, c));

    // Chunk header should be preserved (tcache entry is at user_data, not chunk start)
    ASSERT(c->chunk_size().value == cs);
    ASSERT(c->prev_inuse());

    // Alloc back
    void* p = tcache->alloc(tidx);
    ASSERT(p == c->user_data());

    // Header still intact
    Chunk* c2 = Chunk::from_user_ptr(p);
    ASSERT(c2 == c);
    ASSERT(c2->chunk_size().value == cs);

    free_chunk(c);
    PASS();
}

// ─── Test: invalid index ───

static void test_invalid_index() {
    printf("  invalid_index ... ");

    if (!tcache) tcache_init();

    Chunk* c = make_chunk(64);
    ASSERT(c != nullptr);

    // Out-of-range index
    TcacheIdx bad_idx{TCACHE_MAX_BINS};
    ASSERT(tcache->alloc(bad_idx) == nullptr);
    ASSERT(!tcache->free(bad_idx, c));

    // Very large index
    TcacheIdx huge_idx{9999};
    ASSERT(tcache->alloc(huge_idx) == nullptr);
    ASSERT(!tcache->free(huge_idx, c));

    free_chunk(c);
    PASS();
}

// ─── Test: safe-linking across multiple entries ───

static void test_safe_linking_chain() {
    printf("  safe_linking_chain ... ");

    if (!tcache) tcache_init();

    size_t cs = 64;
    TcacheIdx tidx = csize2tidx(ChunkSize{cs});

    // Build a chain of 5 chunks
    const int N = 5;
    Chunk* chunks[N];
    for (int i = 0; i < N; ++i) {
        chunks[i] = make_chunk(cs);
        ASSERT(chunks[i] != nullptr);
        ASSERT(tcache->free(tidx, chunks[i]));
    }

    // Verify we can alloc all 5 back in correct LIFO order
    for (int i = N - 1; i >= 0; --i) {
        void* p = tcache->alloc(tidx);
        ASSERT(p == chunks[i]->user_data());
    }

    ASSERT(tcache->counts_[tidx.value] == 0);

    for (int i = 0; i < N; ++i) {
        free_chunk(chunks[i]);
    }
    PASS();
}

int main() {
    printf("=== tcache tests ===\n");

    test_safe_linking();
    test_basic_alloc_free();
    test_lifo_order();
    test_fill_count_limit();
    test_multiple_bins();
    test_double_free_detection();
    test_chunk_header_preserved();
    test_invalid_index();
    test_safe_linking_chain();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
