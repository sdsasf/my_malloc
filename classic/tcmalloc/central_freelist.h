#pragma once
// CentralFreeList: per-size-class global free list with batch transfer.
// Protected by a spinlock. Manages non-empty and empty span lists.

#include "size_classes.h"
#include "span.h"

#include <pthread.h>

namespace my_ptmalloc {
namespace tcmalloc {

// Per-size-class central free list
struct CentralFreeList {
    pthread_mutex_t lock;

    // Two span lists:
    Span* nonempty;   // spans with available objects
    Span* empty;       // spans with all objects allocated

    // Batch transfer sizes (slow-start: grows with usage)
    size_t tc_length;  // current batch size for transfers

    CentralFreeList() noexcept;

    // ─── Remove N objects from central list → thread cache ───
    // Returns: head of the transferred freelist, and *count = actual count.
    FreeNode* remove_range(size_t N, size_t* count) noexcept;

    // ─── Insert N freed objects from thread cache → central list ───
    void insert_range(FreeNode* head, FreeNode* tail, size_t N) noexcept;

    // ─── Add a span to this central list ───
    void add_span(Span* span) noexcept;

    // ─── Update slow-start batch length ───
    void update_tc_length(size_t actual) noexcept;
};

// Array of central free lists, indexed by size class
extern CentralFreeList g_central[kNumClasses];

} // namespace tcmalloc
} // namespace my_ptmalloc
