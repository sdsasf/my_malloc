// ArenaManager: arena creation, selection, reuse

#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/thread_registry.h"
#include "my_ptmalloc/tcache.h"
#include "my_ptmalloc/sys_memory.h"
#include "my_ptmalloc/heap.h"
#include <sys/sysinfo.h>
#include <pthread.h>

namespace my_ptmalloc {

ArenaManager::ArenaManager() noexcept {
    pthread_mutex_t init1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t init2 = PTHREAD_MUTEX_INITIALIZER;
    list_lock_ = init1;
    free_list_lock_ = init2;

    // Compute arena limit based on CPU count
    int ncpus = get_nprocs();
    arena_limit_ = (ncpus == 1) ? 1 : ncpus * ARENA_MULTIPLIER;

    // Create default memory source
    static MmapMemory default_mem;
    sys_mem_ = &default_mem;

    // Create default threshold
    static StaticThreshold default_thresh;
    threshold_ = &default_thresh;

    // Initialize main arena
    main_arena_.init(true);
    arena_list_ = &main_arena_;
    main_arena_.next_ = &main_arena_;  // Circular

    // Create top chunk for main arena from mmap
    constexpr size_t INITIAL_HEAP_SIZE = HEAP_MAX_SIZE;
    void* heap = sys_mem_->map(INITIAL_HEAP_SIZE, MALLOC_ALIGNMENT);
    if (heap) {
        Chunk* top = reinterpret_cast<Chunk*>(heap);
        size_t top_size = INITIAL_HEAP_SIZE - MINSIZE;
        top->prev_size = 0;
        top->set_head(ChunkSize{top_size}, ChunkFlag::PREV_INUSE);
        Chunk* fencepost = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(heap) + top_size);
        fencepost->prev_size = top_size;
        fencepost->set_head(ChunkSize{MINSIZE}, ChunkFlag::PREV_INUSE);
        main_arena_.set_top(top);
        main_arena_.update_system_mem(INITIAL_HEAP_SIZE);
    }
}

ArenaManager::~ArenaManager() noexcept = default;

Arena* ArenaManager::get_arena(size_t size) noexcept {
    // If thread already has an arena, use it
    if (thread_arena != nullptr) {
        thread_arena->lock();
        return thread_arena;
    }

    // First allocation for this thread
    ThreadRegistry::register_thread();

    Arena* av = nullptr;
    int cur = narenas_.load(std::memory_order_acquire);
    while (cur < arena_limit_) {
        if (narenas_.compare_exchange_weak(
                cur, cur + 1,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
            av = new_arena(size);
            if (!av) narenas_.fetch_sub(1, std::memory_order_acq_rel);
            break;
        }
    }

    if (!av) {
        av = get_free_list();
    }
    if (!av) {
        av = reused_arena(nullptr);
        av->attached_threads_++;
        thread_arena = av;
        return av;
    }

    av->attached_threads_++;
    thread_arena = av;
    av->lock();
    return av;
}

Arena* ArenaManager::get_free_list() noexcept {
    pthread_mutex_lock(&free_list_lock_);
    Arena* av = free_list_;
    if (av) {
        free_list_ = av->next_free_;
        av->next_free_ = nullptr;
    }
    pthread_mutex_unlock(&free_list_lock_);
    return av;
}

Arena* ArenaManager::new_arena(size_t) noexcept {
    // Allocate heap for new arena (pass nullptr for arena ptr — set after arena is placed)
    size_t heap_size = HEAP_MAX_SIZE;
    HeapInfo* h = HeapInfo::new_heap(heap_size, nullptr, nullptr);
    if (!h) return nullptr;

    // Place arena struct after heap info
    uintptr_t addr = reinterpret_cast<uintptr_t>(h) + sizeof(HeapInfo);
    addr = (addr + 127) & ~127;  // alignas(128)
    Arena* av = reinterpret_cast<Arena*>(addr);
    av->init(false);

    // Wire up HeapInfo → Arena link
    h->ar_ptr = av;

    // Create top chunk from remaining space
    uintptr_t top_addr = addr + sizeof(Arena);
    top_addr = (top_addr + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;
    size_t top_size = heap_size - (top_addr - reinterpret_cast<uintptr_t>(h));
    if (top_size >= 2 * MINSIZE) top_size -= MINSIZE;
    if (top_size >= MINSIZE) {
        Chunk* top = reinterpret_cast<Chunk*>(top_addr);
        top->prev_size = 0;
        top->set_head(ChunkSize{top_size}, ChunkFlag::PREV_INUSE);
        Chunk* fencepost = reinterpret_cast<Chunk*>(top_addr + top_size);
        fencepost->prev_size = top_size;
        fencepost->set_head(ChunkSize{MINSIZE}, ChunkFlag::PREV_INUSE | ChunkFlag::NON_MAIN_ARENA);
        av->set_top(top);
        av->update_system_mem(top_size);
    }

    // Link into arena list
    pthread_mutex_lock(&list_lock_);
    av->next_ = arena_list_->next_;
    arena_list_->next_ = av;
    pthread_mutex_unlock(&list_lock_);

    return av;
}

Arena* ArenaManager::reused_arena(Arena*) noexcept {
    // Round-robin trylock across all arenas
    pthread_mutex_lock(&list_lock_);
    Arena* av = arena_list_;
    Arena* first = av;
    pthread_mutex_unlock(&list_lock_);

    do {
        if (av->trylock()) {
            return av;
        }
        av = av->next_;
    } while (av != first);

    // All arenas contended, block on first
    av = arena_list_;
    av->lock();
    return av;
}

} // namespace my_ptmalloc
