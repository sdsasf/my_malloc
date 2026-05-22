#pragma once
// Per-thread cache (tcache) — glibc 2.26+ style.
// Lock-free LIFO singly-linked lists, 64 bins, max 7 entries per bin.

#include "config.h"
#include "chunk.h"

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Tcache entry (overlays user data) ───
struct TcacheEntry {
    TcacheEntry* next;     // Protected by per-bin count checks (not CAS)
    // key field stores tcache* pointer for double-free detection
    Chunk* chunk_key;      // points back to the chunk containing this entry
};

// ─── Per-thread tcache ───
// Each bin is a singly-linked LIFO freelist.
// entries[i]: count of available entries in bin i
struct Tcache {
    TcacheEntry* bins[TCACHE_MAX_BINS];
    unsigned     count[TCACHE_MAX_BINS];

    Tcache() noexcept;

    // ─── Alloc ───
    // Pop from bin `tidx`. Returns user-data pointer or nullptr.
    [[nodiscard]] void* alloc(TcacheIdx tidx) noexcept;

    // ─── Free ───
    // Push chunk to bin `tidx`. Returns true on success, false if bin full.
    bool free(TcacheIdx tidx, Chunk* chunk) noexcept;

    // ─── Flush ───
    // Drain all entries from bin `tidx`, return as singly-linked freelist head.
    [[nodiscard]] TcacheEntry* flush(TcacheIdx tidx) noexcept;

    // ─── Double-free check ───
    bool is_double_free(TcacheEntry* e) const noexcept;

    // ─── Total entries across all bins ───
    size_t total_count() const noexcept;
};

// Thread-local tcache pointer
extern thread_local Tcache* tcache;

// Initialize thread-local tcache
void tcache_init() noexcept;

} // namespace ptmalloc
} // namespace my_ptmalloc
