// tcmalloc core implementation.
// Three-layer architecture: ThreadCache → CentralFreeList → PageHeap.

#include "tcmalloc.h"
#include "thread_cache.h"
#include "central_freelist.h"
#include "page_heap.h"

#include <pthread.h>
#include <cstring>
#include <sys/mman.h>

namespace my_ptmalloc {
namespace tcmalloc {

static pthread_once_t tc_once = PTHREAD_ONCE_INIT;

void tc_init_allocator() noexcept {
    pthread_once(&tc_once, []() {
        tc_init();
    });
}

// ─── Large allocation (> kMaxSize = 256KB) ───
static void* tc_large_alloc(size_t size) noexcept {
    Span* span = g_page_heap.alloc_large(size);
    if (!span) return nullptr;
    span->size_class = kNumClasses;  // mark as large
    span->refcount = 1;
    return Span::page_addr(span->start);
}

static void tc_large_free(void* ptr) noexcept {
    uintptr_t pn = Span::page_number(ptr);
    Span* span = g_page_heap.pagemap.get(pn);
    if (!span) return;
    void* base = Span::page_addr(span->start);
    size_t map_size = span->length * kPageSize;
    ::munmap(base, map_size);
    g_page_heap.system_bytes -= map_size;
    delete span;
}

static size_t tc_large_usable(void* ptr) noexcept {
    uintptr_t pn = Span::page_number(ptr);
    Span* span = g_page_heap.pagemap.get(pn);
    return span ? span->length * kPageSize : 0;
}

// ─── Small allocation ───
void* tc_malloc(size_t size) noexcept {
    tc_init_allocator();
    if (size == 0) size = 1;

    // Large allocation
    if (size > kMaxSize) {
        return tc_large_alloc(size);
    }

    unsigned class_idx = size_to_class(size);
    ThreadCache* tc = tc_thread_cache;
    if (!tc) return nullptr;

    void* p = tc->alloc(class_idx);
    if (p) return p;

    // Thread cache couldn't satisfy — go directly to central
    size_t batch = kClassInfo[class_idx].num_to_move;
    size_t got = 0;
    FreeNode* head = g_central[class_idx].remove_range(1, &got);
    if (!head) {
        // Need a new span from PageHeap
        size_t pages = kClassInfo[class_idx].pages;
        Span* span = g_page_heap.alloc_span(pages);
        if (!span) return nullptr;

        span->size_class = class_idx;
        span->build_freelist(kClassInfo[class_idx].size);
        g_central[class_idx].add_span(span);

        head = g_central[class_idx].remove_range(1, &got);
        if (!head) return nullptr;
    }

    return static_cast<void*>(head);
}

// ─── Free ───
void tc_free(void* ptr) noexcept {
    if (!ptr) return;

    // Check if large
    uintptr_t pn = Span::page_number(ptr);
    Span* span = g_page_heap.pagemap.get(pn);
    if (!span) return;

    if (span->size_class >= kNumClasses) {
        tc_large_free(ptr);
        return;
    }

    // Small object
    unsigned class_idx = span->size_class;
    ThreadCache* tc = tc_thread_cache;
    if (tc) {
        if (tc->free(class_idx, ptr)) return;
    }

    // Direct to central freelist
    auto* node = static_cast<FreeNode*>(ptr);
    node->next = nullptr;
    g_central[class_idx].insert_range(node, node, 1);

    span->refcount--;
    if (span->refcount == 0) {
        g_page_heap.free_span(span);
    }
}

// ─── Calloc ───
void* tc_calloc(size_t n, size_t size) noexcept {
    size_t total = n * size;
    if (n != 0 && total / n != size) return nullptr;
    void* p = tc_malloc(total);
    if (p) std::memset(p, 0, tc_usable_size(p) < total ? tc_usable_size(p) : total);
    return p;
}

// ─── Realloc ───
void* tc_realloc(void* ptr, size_t size) noexcept {
    if (!ptr) return tc_malloc(size);
    if (size == 0) { tc_free(ptr); return nullptr; }

    size_t old_size = tc_usable_size(ptr);
    if (old_size >= size) return ptr;

    void* np = tc_malloc(size);
    if (!np) return nullptr;
    std::memcpy(np, ptr, old_size < size ? old_size : size);
    tc_free(ptr);
    return np;
}

// ─── Memalign ───
void* tc_memalign(size_t alignment, size_t size) noexcept {
    if (alignment <= 16) return tc_malloc(size);
    if ((alignment & (alignment - 1)) != 0) return nullptr;
    // Use large allocation with alignment
    void* p = tc_large_alloc(size + alignment);
    if (!p) return nullptr;
    return reinterpret_cast<void*>(
        (reinterpret_cast<uintptr_t>(p) + alignment - 1) & ~(alignment - 1));
}

// ─── Usable size ───
size_t tc_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;
    uintptr_t pn = Span::page_number(ptr);
    Span* span = g_page_heap.pagemap.get(pn);
    if (!span) return 0;
    if (span->size_class >= kNumClasses) return tc_large_usable(ptr);
    return kClassInfo[span->size_class].size;
}

} // namespace tcmalloc
} // namespace my_ptmalloc
