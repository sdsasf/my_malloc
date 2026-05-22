#pragma once
// ThreadCache: per-thread lock-free cache (tcmalloc style).
// Each size class has its own freelist with slow-start capacity management.

#include "size_classes.h"
#include "central_freelist.h"

#include <cstddef>

namespace my_ptmalloc {
namespace tcmalloc {

struct ThreadCache {
    // Per-size-class freelist
    FreeNode* list[kNumClasses];
    size_t    count[kNumClasses];      // entries in this bin
    size_t    max_count[kNumClasses];   // capacity (slow-start growth)
    size_t    low_water[kNumClasses];   // when to refill

    // Total allocated bytes (for periodic GC)
    size_t    allocated;
    size_t    max_allocated;    // threshold before GC triggered

    // Next in global thread cache list
    ThreadCache* next;
    ThreadCache* prev;

    ThreadCache() noexcept;

    // ─── Allocate from thread cache ───
    void* alloc(size_t class_idx) noexcept;

    // ─── Free to thread cache ───
    bool free(size_t class_idx, void* ptr) noexcept;

    // ─── GC: reclaim objects to central freelist ───
    void scavenge() noexcept;

    // ─── Refill from central freelist ───
    void refill(size_t class_idx) noexcept;

    // ─── Adjust slow-start capacity ───
    void update_capacity(size_t class_idx) noexcept;
};

// Thread-local thread cache
extern thread_local ThreadCache* tc_thread_cache;

// Initialize thread-local cache
void tc_init() noexcept;

} // namespace tcmalloc
} // namespace my_ptmalloc
