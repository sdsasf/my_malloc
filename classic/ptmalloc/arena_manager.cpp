// ArenaManager implementation — multi-arena lifecycle.

#include "arena_manager.h"
#include "heap.h"
#include "sys_memory.h"

#include <cstring>
#include <new>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Thread-local arena ───
thread_local Arena* thread_arena = nullptr;

// ─── Global singleton ───
ArenaManager* g_arena_manager = nullptr;

// Global arena head for heap registry
Arena* g_all_arenas_head = nullptr;

// ─── ArenaManager ───

ArenaManager::ArenaManager() noexcept
    : free_arena_list_(nullptr)
    , narenas_(0)
    , next_to_use_(0)
{
    arena_list_lock_ = PTHREAD_MUTEX_INITIALIZER;
}

void ArenaManager::init() noexcept {
    // Initialize main arena
    main_arena_.init(true);
    narenas_.store(1, std::memory_order_release);
    g_all_arenas_head = &main_arena_;

    // Create initial top chunk for main arena via mmap
    size_t heap_size = HEAP_MAX_SIZE;
    void* heap = sysmem_.map(heap_size);
    if (heap) {
        // Set up the heap as the initial top chunk
        Chunk* top = static_cast<Chunk*>(heap);
        top->prev_size = 0;
        top->set_head(ChunkSize{heap_size - MINSIZE}, ChunkFlag::PREV_INUSE);

        // Create fencepost at the end
        Chunk* fence = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(heap) + heap_size - MINSIZE);
        fence->prev_size = heap_size - MINSIZE;
        fence->set_head(ChunkSize{MINSIZE}, ChunkFlag::PREV_INUSE);

        main_arena_.set_top(top);
        main_arena_.update_system_mem(heap_size);
    }
}

Arena* ArenaManager::get_arena(size_t nb) noexcept {
    // For large mmap allocations, use the arena to coordinate but don't
    // actually need arena bins — the allocation path will use mmap directly.
    // We still need a valid arena to hold the lock.

    // Check if thread already has an arena
    Arena* av = thread_arena;
    if (av && !av->is_main_ && av->trylock()) {
        // Successfully locked our existing arena — use it
        (void)nb; // actual allocation done by caller
        return av;
    }

    // Try main arena if convenient
    if (main_arena_.trylock()) {
        return &main_arena_;
    }

    // Contention: need a different arena
    // Try the free list
    av = get_free_arena();
    if (!av) {
        av = new_arena();
    }
    if (av) {
        av->lock();
        thread_arena = av;
        return av;
    }

    // Fallback to main arena (blocking)
    main_arena_.lock();
    return &main_arena_;
}

Arena* ArenaManager::get_free_arena() noexcept {
    pthread_mutex_lock(&arena_list_lock_);
    Arena* av = free_arena_list_;
    if (av) {
        free_arena_list_ = av->next_free_;
        av->next_free_ = nullptr;
    }
    pthread_mutex_unlock(&arena_list_lock_);
    return av;
}

Arena* ArenaManager::new_arena() noexcept {
    unsigned n = narenas_.load(std::memory_order_acquire);
    if (n >= MAX_ARENAS) return nullptr;

    // Allocate arena structure on its own page
    void* mem = sysmem_.map(sizeof(Arena));
    if (!mem) return nullptr;

    auto* av = new (mem) Arena();
    av->init(false);

    // Add to global arena list
    pthread_mutex_lock(&arena_list_lock_);
    av->next_ = g_all_arenas_head;
    g_all_arenas_head = av;
    narenas_.fetch_add(1, std::memory_order_release);
    pthread_mutex_unlock(&arena_list_lock_);

    // Create initial HeapInfo + top chunk for this arena
    HeapInfo* h = HeapInfo::new_heap(HEAP_MAX_SIZE, av, &sysmem_);
    if (h) {
        uintptr_t chunk_start = reinterpret_cast<uintptr_t>(h) + sizeof(HeapInfo);
        chunk_start = (chunk_start + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;

        size_t usable = reinterpret_cast<uintptr_t>(h) + h->size - chunk_start;
        if (usable < MINSIZE * 2) {
            // Not enough room — arena will need heap extension on first use
            av->set_top(nullptr);
            av->update_system_mem(h->size);
        } else {
            Chunk* top = reinterpret_cast<Chunk*>(chunk_start);
            top->prev_size = 0;

            // Leave room for fencepost
            size_t top_size = usable - MINSIZE;
            top->set_head(ChunkSize{top_size},
                ChunkFlag::PREV_INUSE | ChunkFlag::NON_MAIN_ARENA);

            Chunk* fence = reinterpret_cast<Chunk*>(chunk_start + top_size);
            fence->prev_size = top_size;
            fence->set_head(ChunkSize{MINSIZE},
                ChunkFlag::PREV_INUSE | ChunkFlag::NON_MAIN_ARENA);

            av->set_top(top);
            av->update_system_mem(h->size);
        }
    }

    return av;
}

Arena* ArenaManager::reuse_arena(Arena* avoided) noexcept {
    (void)avoided;
    return get_free_arena();
}

unsigned ArenaManager::arena_count() const noexcept {
    return narenas_.load(std::memory_order_acquire);
}

} // namespace ptmalloc
} // namespace my_ptmalloc
