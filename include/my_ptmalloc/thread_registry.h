#pragma once
// ThreadRegistry: explicit thread lifecycle management
// Handles arena assignment and tcache init/shutdown per thread

#include <cstddef>

namespace my_ptmalloc {

struct Arena;

enum class ThreadState {
    UNREGISTERED,
    ACTIVE,
    SHUTTING_DOWN,
    DETACHED
};

// Thread-local state
inline thread_local Arena*       thread_arena = nullptr;
inline thread_local ThreadState  thread_state = ThreadState::UNREGISTERED;

class ThreadRegistry {
public:
    // Called on first allocation by a new thread
    static void register_thread() noexcept;

    // Called on thread exit
    static void unregister_thread() noexcept;

    // Fork safety
    static void on_fork_parent() noexcept;
    static void on_fork_child() noexcept;
};

} // namespace my_ptmalloc
