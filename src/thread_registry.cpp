// ThreadRegistry: thread lifecycle management

#include "my_ptmalloc/thread_registry.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/tcache.h"
#include "my_ptmalloc/arena_manager.h"

namespace my_ptmalloc {

void ThreadRegistry::register_thread() noexcept {
    if (thread_state == ThreadState::ACTIVE) return;
    thread_state = ThreadState::ACTIVE;

    // Initialize tcache for this thread
    tcache_init();

    // Arena assignment happens lazily on first allocation
}

void ThreadRegistry::unregister_thread() noexcept {
    if (thread_state != ThreadState::ACTIVE) return;
    thread_state = ThreadState::SHUTTING_DOWN;

    // Flush tcache to arena (must hold arena lock for bin modifications)
    if (thread_arena && tcache) {
        thread_arena->lock();
        tcache_shutdown(*thread_arena);
        thread_arena->unlock();
    }

    // Detach from arena
    if (thread_arena) {
        thread_arena->attached_threads_--;
        if (thread_arena->attached_threads_ == 0) {
            // Arena is now free, could be reused
            // For now, just leave it
        }
        thread_arena = nullptr;
    }

    thread_state = ThreadState::DETACHED;
}

void ThreadRegistry::on_fork_parent() noexcept {
    // Lock all arenas before fork
    // Simplified: just lock main arena
    main_arena.lock();
}

void ThreadRegistry::on_fork_child() noexcept {
    // Child process: reinitialize locks
    main_arena.unlock();
    main_arena.init(true);
}

} // namespace my_ptmalloc
