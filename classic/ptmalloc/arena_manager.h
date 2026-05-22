#pragma once
// ArenaManager: multi-arena lifecycle, thread-to-arena assignment.
// Faithful to glibc's arena_get / arena management.

#include "config.h"
#include "arena.h"
#include "chunk.h"
#include "sys_memory.h"

#include <pthread.h>
#include <atomic>

namespace my_ptmalloc {
namespace ptmalloc {

// Forward decls
struct ThresholdPolicy;

// ─── Threshold policy (mmap threshold + trim threshold) ───
struct ThresholdPolicy {
    virtual ~ThresholdPolicy() = default;
    virtual size_t   mmap_threshold() const noexcept = 0;
    virtual void     set_mmap_threshold(size_t t) noexcept = 0;
    virtual size_t   trim_threshold() const noexcept = 0;
    virtual void     set_trim_threshold(size_t t) noexcept = 0;
    virtual void     adjust_mmap_threshold(bool is_mmap) noexcept = 0;
};

struct StaticThreshold : ThresholdPolicy {
    size_t mmap_thresh_;
    size_t trim_thresh_;

    StaticThreshold() noexcept
        : mmap_thresh_(DEFAULT_MMAP_THRESHOLD)
        , trim_thresh_(DEFAULT_TRIM_THRESHOLD) {}

    size_t mmap_threshold() const noexcept override { return mmap_thresh_; }
    void set_mmap_threshold(size_t t) noexcept override { mmap_thresh_ = t; }
    size_t trim_threshold() const noexcept override { return trim_thresh_; }
    void set_trim_threshold(size_t t) noexcept override { trim_thresh_ = t; }
    void adjust_mmap_threshold(bool is_mmap) noexcept override {
        if (is_mmap) {
            // If we mmap'd, raise the threshold to avoid future mmaps
            if (mmap_thresh_ < DEFAULT_MMAP_THRESHOLD_MAX) {
                mmap_thresh_ += (mmap_thresh_ >> 2);  // grow by 25%
                if (mmap_thresh_ > DEFAULT_MMAP_THRESHOLD_MAX)
                    mmap_thresh_ = DEFAULT_MMAP_THRESHOLD_MAX;
            }
        } else {
            // If we used the heap, lower the threshold to prefer heap
            if (mmap_thresh_ > DEFAULT_MMAP_THRESHOLD_MIN) {
                mmap_thresh_ -= (mmap_thresh_ >> 3);  // shrink by 12.5%
                if (mmap_thresh_ < DEFAULT_MMAP_THRESHOLD_MIN)
                    mmap_thresh_ = DEFAULT_MMAP_THRESHOLD_MIN;
            }
        }
    }
};

// ─── ArenaManager ───
class ArenaManager {
public:
    ArenaManager() noexcept;
    ~ArenaManager() noexcept = default;

    // Initialize on first use
    void init() noexcept;

    // Get an arena for allocation of `nb` bytes.
    // Returns with arena LOCKED.
    Arena* get_arena(size_t nb) noexcept;

    // Get main arena (always available)
    Arena* get_main_arena() noexcept { return &main_arena_; }

    // SysMemory accessor
    SysMemory* sys_memory() noexcept { return &sysmem_; }

    // Threshold policy
    ThresholdPolicy* threshold() noexcept { return &threshold_; }

    // Total arena count (main + non-main)
    unsigned arena_count() const noexcept;

private:
    Arena           main_arena_;
    SysMemory       sysmem_;
    StaticThreshold threshold_;

    pthread_mutex_t arena_list_lock_;
    Arena*          free_arena_list_;       // recycled arenas
    std::atomic<unsigned> narenas_;         // total # of arenas
    std::atomic<unsigned> next_to_use_;     // round-robin assignment

    // Get a free arena from the list or create a new one
    Arena* get_free_arena() noexcept;

    // Create a new non-main arena
    Arena* new_arena() noexcept;

    // Try to reuse an existing arena
    Arena* reuse_arena(Arena* avoided) noexcept;
};

// Thread-local pointer to the arena currently assigned to this thread
extern thread_local Arena* thread_arena;

// Global arena manager singleton
extern ArenaManager* g_arena_manager;

} // namespace ptmalloc
} // namespace my_ptmalloc
