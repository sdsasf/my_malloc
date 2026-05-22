#pragma once
// PageHeap: manages spans of pages. Allocates spans from OS, maintains
// free lists per span-size, and provides spans to CentralFreeList.

#include "span.h"
#include "page_map.h"

#include <pthread.h>

namespace my_ptmalloc {
namespace tcmalloc {

struct PageHeap {
    pthread_mutex_t lock;

    // Free span lists: one per page count (1..kMaxPages)
    Span* free_[kMaxPages + 1];

    // Large spans (>kMaxPages): stored in a simple set
    Span* large_set;

    // PageMap: maps page number → Span*
    PageMap pagemap;

    // Stats
    size_t system_bytes;       // total bytes from OS
    size_t free_bytes;         // bytes in free spans

    PageHeap() noexcept;

    // ─── Allocate a span of `n` pages ───
    Span* alloc_span(size_t n) noexcept;

    // ─── Free a span back to the page heap ───
    void free_span(Span* span) noexcept;

    // ─── Get a span for a large (>kMaxSize) allocation ───
    Span* alloc_large(size_t bytes) noexcept;

    // ─── Free a large span ───
    void free_large(Span* span) noexcept;

    // ─── Grow from OS ───
    Span* grow_heap(size_t n) noexcept;

    // ─── Try to coalesce with adjacent free spans ───
    Span* coalesce(Span* span) noexcept;

    // ─── Return excess memory to OS ───
    void release_to_os() noexcept;
};

extern PageHeap g_page_heap;

} // namespace tcmalloc
} // namespace my_ptmalloc
