// Tcache implementation

#include "my_ptmalloc/tcache.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/chunk.h"
#include <new>
#include <atomic>

namespace my_ptmalloc {

// Static buffer for bootstrap tcache (before we can allocate)
static char tcache_bootstrap_buf[sizeof(TcachePerthread) + 64];
static std::atomic<bool> tcache_bootstrap_used{false};

void tcache_init() noexcept {
    if (tcache != nullptr) return;

    bool expected = false;
    if (tcache_bootstrap_used.compare_exchange_strong(expected, true,
            std::memory_order_acq_rel, std::memory_order_acquire)) {
        // Use bootstrap buffer for first thread
        void* buf = tcache_bootstrap_buf;
        // Align to 64 bytes
        uintptr_t addr = reinterpret_cast<uintptr_t>(buf);
        addr = (addr + 63) & ~63;
        tcache = new (reinterpret_cast<void*>(addr)) TcachePerthread();
    } else {
        // For subsequent threads, use placement new on a static buffer
        // (We can't call malloc yet - that would recurse)
        static thread_local char per_thread_buf[sizeof(TcachePerthread) + 64];
        uintptr_t addr = reinterpret_cast<uintptr_t>(per_thread_buf);
        addr = (addr + 63) & ~63;
        tcache = new (reinterpret_cast<void*>(addr)) TcachePerthread();
    }

    tcache->init_key();
}

void tcache_shutdown(Arena& arena) noexcept {
    if (tcache == nullptr) return;

    // Flush all tcache bins back to arena
    tcache->flush_all(arena);
    tcache = nullptr;
}

void TcachePerthread::flush_all(Arena& arena) noexcept {
    for (size_t i = 0; i < TCACHE_MAX_BINS; ++i) {
        TcacheEntry* e = entries_[i];
        entries_[i] = nullptr;
        counts_[i] = 0;

        while (e != nullptr) {
            TcacheEntry* next = reveal_ptr(e, e->next);
            // TcacheEntry sits at user data (chunk + CHUNK_HDR_SZ).
            // Convert back to Chunk* for bin operations.
            Chunk* p = reinterpret_cast<Chunk*>(
                reinterpret_cast<uintptr_t>(e) - CHUNK_HDR_SZ);

            // Clear stale tcache entry data before push.
            // push_front will overwrite fd/bk with proper list pointers,
            // but clearing prevents any window where stale safe-linked
            // pointers could confuse consolidation integrity checks.
            e->next = nullptr;
            e->key = nullptr;

            // Place chunk into unsorted bin
            arena.bins_.unsorted().push(p);
            e = next;
        }
    }
}

} // namespace my_ptmalloc
