// Per-thread tcache implementation.

#include "tcache.h"

#include <cstdlib>  // aligned_alloc for tcache object
#include <new>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Thread-local tcache ───
thread_local Tcache* tcache = nullptr;

Tcache::Tcache() noexcept {
    for (unsigned i = 0; i < TCACHE_MAX_BINS; ++i) {
        bins[i] = nullptr;
        count[i] = 0;
    }
}

void tcache_init() noexcept {
    if (tcache) return;
    // Allocate tcache object aligned to cache line
    void* mem = std::aligned_alloc(64, sizeof(Tcache));
    if (!mem) return;
    tcache = new (mem) Tcache();
}

void* Tcache::alloc(TcacheIdx tidx) noexcept {
    unsigned idx = tidx.value;
    if (idx >= TCACHE_MAX_BINS || !bins[idx]) return nullptr;

    TcacheEntry* e = bins[idx];
    bins[idx] = e->next;
    count[idx]--;

    // Clear key for double-free detection
    e->chunk_key = nullptr;

    return Chunk::from_user_ptr(static_cast<void*>(e))->user_data();
}

bool Tcache::free(TcacheIdx tidx, Chunk* chunk) noexcept {
    unsigned idx = tidx.value;
    if (idx >= TCACHE_MAX_BINS) return false;
    if (count[idx] >= TCACHE_FILL_COUNT) return false;  // bin full

    auto* e = reinterpret_cast<TcacheEntry*>(chunk->user_data());
    e->next = bins[idx];
    e->chunk_key = chunk;  // store key for double-free detection
    bins[idx] = e;
    count[idx]++;
    return true;
}

TcacheEntry* Tcache::flush(TcacheIdx tidx) noexcept {
    unsigned idx = tidx.value;
    if (idx >= TCACHE_MAX_BINS) return nullptr;

    TcacheEntry* head = bins[idx];
    bins[idx] = nullptr;
    count[idx] = 0;

    // Clear keys on all flushed entries
    for (TcacheEntry* e = head; e; e = e->next) {
        e->chunk_key = nullptr;
    }
    return head;
}

bool Tcache::is_double_free(TcacheEntry* e) const noexcept {
    // Check if this entry's key already points to a tcache entry
    // (simplified: check if the key is non-null, matching the chunk)
    return e->chunk_key != nullptr;
}

size_t Tcache::total_count() const noexcept {
    size_t n = 0;
    for (unsigned i = 0; i < TCACHE_MAX_BINS; ++i) {
        n += count[i];
    }
    return n;
}

} // namespace ptmalloc
} // namespace my_ptmalloc
