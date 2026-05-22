#pragma once
// Arena: per-arena structure with bins, top chunk, and lock.

#include "config.h"
#include "bins.h"
#include "chunk.h"

#include <pthread.h>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Arena ───
// Each arena has its own bin collection, top chunk, and mutex.
// Main arena (index 0) grows via mmap'd heaps.
// Non-main arenas grow via HeapInfo-linked mmap'd heaps.

struct Arena {
    pthread_mutex_t mutex_;
    BinManager      bins_;
    Chunk*          top_;            // top (wilderness) chunk
    Chunk*          last_remainder_;  // cached remainder from last split

    // Linked list of arenas
    Arena*          next_;
    Arena*          next_free_;      // free list for arena reuse

    // Arena metadata
    unsigned        attached_threads_;
    size_t          system_mem_;      // total memory allocated to this arena
    size_t          max_system_mem_;
    bool            is_main_;

    Arena() noexcept;
    void init(bool is_main) noexcept;

    void lock() noexcept   { pthread_mutex_lock(&mutex_); }
    void unlock() noexcept { pthread_mutex_unlock(&mutex_); }
    bool trylock() noexcept { return pthread_mutex_trylock(&mutex_) == 0; }

    // ─── Top chunk operations ───
    Chunk* top() const noexcept { return top_; }
    void set_top(Chunk* p) noexcept { top_ = p; }
    void set_last_remainder(Chunk* p) noexcept { last_remainder_ = p; }

    // Check if arena has chunks in fastbins (need consolidation)
    bool has_fastchunks() const noexcept { return bins_.has_fastchunks(); }

    // Update system memory tracking
    void update_system_mem(size_t added) noexcept {
        system_mem_ += added;
        if (system_mem_ > max_system_mem_) max_system_mem_ = system_mem_;
    }
};

// ─── ArenaGuard (RAII lock) ───
struct ArenaGuard {
    Arena* arena_;
    explicit ArenaGuard(Arena* av) noexcept : arena_(av) {
        if (arena_) arena_->lock();
    }
    ~ArenaGuard() noexcept {
        if (arena_) arena_->unlock();
    }
};

} // namespace ptmalloc
} // namespace my_ptmalloc
