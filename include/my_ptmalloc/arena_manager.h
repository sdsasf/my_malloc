#pragma once
// ArenaManager: arena creation, selection, and reuse
// Manages the circular linked list of arenas and the free list

#include "config.h"
#include "arena.h"
#include "sys_memory.h"
#include "threshold.h"
#include <atomic>
#include <pthread.h>

namespace my_ptmalloc {

class ArenaManager {
    Arena       main_arena_;
    Arena*      arena_list_ = nullptr;     // Circular linked list
    Arena*      free_list_ = nullptr;      // Detached arenas for reuse
    std::atomic<int> narenas_{1};
    int         arena_limit_;
    pthread_mutex_t list_lock_;
    pthread_mutex_t free_list_lock_;
    SysMemory*      sys_mem_;
    ThresholdPolicy* threshold_;

public:
    ArenaManager() noexcept;
    ~ArenaManager() noexcept;

    // Get arena for current thread (may create new one)
    Arena* get_arena(size_t size) noexcept;

    // Get main arena directly
    [[nodiscard]] Arena* get_main_arena() noexcept { return &main_arena_; }

    // System memory accessor
    [[nodiscard]] SysMemory* sys_memory() noexcept { return sys_mem_; }

    // Threshold accessor
    [[nodiscard]] ThresholdPolicy* threshold() noexcept { return threshold_; }

    // Set custom sys memory (for testing)
    void set_sys_memory(SysMemory* mem) noexcept { sys_mem_ = mem; }

    // Set custom threshold policy
    void set_threshold(ThresholdPolicy* t) noexcept { threshold_ = t; }

private:
    Arena* get_free_list() noexcept;
    Arena* new_arena(size_t size) noexcept;
    Arena* reused_arena(Arena* avoid) noexcept;
};

extern ArenaManager* g_arena_manager;

} // namespace my_ptmalloc
