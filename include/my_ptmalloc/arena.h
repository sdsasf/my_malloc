#pragma once
// Arena: the core allocation state
// Cache-line aligned to prevent false sharing between threads
// Composed of independent bin subsystems (not flat array)

#include "config.h"
#include "types.h"
#include "chunk.h"
#include "bin_manager.h"
#include <pthread.h>
#include <cstddef>
#include <atomic>

namespace my_ptmalloc {

// RAII arena lock guard
class ArenaGuard {
    Arena* arena_;
public:
    explicit ArenaGuard(Arena* av) noexcept;
    ~ArenaGuard() noexcept;
    ArenaGuard(const ArenaGuard&) = delete;
    ArenaGuard& operator=(const ArenaGuard&) = delete;

    [[nodiscard]] Arena* get() const noexcept { return arena_; }
    [[nodiscard]] Arena* operator->() const noexcept { return arena_; }
};

struct alignas(128) Arena {
    // ─── Hot path data ───
    Chunk*       top_ = nullptr;          // Top (wilderness) chunk
    Chunk*       last_remainder_ = nullptr; // Remainder from last split

    // Bin subsystem (composed, not flat array)
    BinManager   bins_;

    // ─── Warm data ───
    unsigned int flags_ = 0;              // FASTCHUNKS_BIT
    Arena*       next_ = nullptr;         // Circular linked list of all arenas
    Arena*       next_free_ = nullptr;    // Free list for detached arenas

    // ─── Cold data (separate cache line group) ───
    alignas(64) pthread_mutex_t mutex_;
    size_t       attached_threads_ = 0;
    size_t       system_mem_ = 0;         // Current total system memory
    size_t       max_system_mem_ = 0;     // Peak system memory

    // Initialize arena
    void init(bool is_main) noexcept;

    // Lock operations
    void lock() noexcept { pthread_mutex_lock(&mutex_); }
    void unlock() noexcept { pthread_mutex_unlock(&mutex_); }
    bool trylock() noexcept { return pthread_mutex_trylock(&mutex_) == 0; }

    // Top chunk operations
    [[nodiscard]] Chunk* top() const noexcept { return top_; }
    void set_top(Chunk* p) noexcept { top_ = p; }

    // last_remainder operations
    [[nodiscard]] Chunk* last_remainder() const noexcept { return last_remainder_; }
    void set_last_remainder(Chunk* p) noexcept { last_remainder_ = p; }

    // Flags
    [[nodiscard]] bool has_fastchunks() const noexcept {
        return (flags_ & FASTCHUNKS_BIT) != 0;
    }
    void set_fastchunks() noexcept { flags_ |= FASTCHUNKS_BIT; }
    void clear_fastchunks() noexcept { flags_ &= ~FASTCHUNKS_BIT; }

    // Memory accounting
    void update_system_mem(size_t bytes) noexcept {
        system_mem_ += bytes;
        if (system_mem_ > max_system_mem_) max_system_mem_ = system_mem_;
    }
};

// Consolidate fastbins into unsorted bin (with merging)
void malloc_consolidate(Arena& arena) noexcept;

// Trim top chunk: release memory beyond pad bytes back to OS.
// Returns true if any memory was released.
bool systrim(Arena& arena, size_t pad) noexcept;

// Main arena (static global)
extern Arena main_arena;

} // namespace my_ptmalloc
