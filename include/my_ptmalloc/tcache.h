#pragma once
// Tcache: per-thread cache for fast allocation/deallocation
// Singly-linked LIFO lists, up to 16 chunks per bin, 76 bins total
// Lock-free: entirely thread-local, zero contention
// Safe-linking: pointer mangling to mitigate heap exploitation

#include "config.h"
#include "types.h"
#include "chunk.h"
#include <cstdint>
#include <random>

namespace my_ptmalloc {

struct TcachePerthread;  // Forward declaration

struct TcacheEntry {
    TcacheEntry* next;          // Safe-linked pointer
    TcachePerthread* key;       // For double-free detection
};

struct alignas(64) TcachePerthread {
    uint16_t     counts_[TCACHE_MAX_BINS]{};
    TcacheEntry* entries_[TCACHE_MAX_BINS]{};
    uint64_t     random_key_;   // For double-free detection + safe-linking entropy

    // Initialize random key for safe-linking and double-free detection
    void init_key() noexcept {
        std::random_device rd;
        random_key_ = (static_cast<uint64_t>(rd()) << 32) | rd();
        if (random_key_ == 0) random_key_ = 1;  // Avoid zero key
        // Init entries
        for (auto& e : entries_) e = nullptr;
        for (auto& c : counts_) c = 0;
    }

    // Allocate from tcache (lock-free fast path)
    // Returns user data pointer (chunk + CHUNK_HDR_SZ)
    [[nodiscard]] void* alloc(TcacheIdx idx) noexcept {
        if (idx.value >= TCACHE_MAX_BINS) return nullptr;
        if (counts_[idx.value] == 0) return nullptr;

        TcacheEntry* e = entries_[idx.value];
        // Reveal safe-linked pointer using position-based XOR.
        // Position is `e` (where e->next is stored).
        entries_[idx.value] = reveal_ptr(e, e->next);
        counts_[idx.value]--;
        // Clear stale double-free key in returned entry to avoid false positives.
        e->key = nullptr;

        // TcacheEntry sits at user data location (chunk + CHUNK_HDR_SZ),
        // same as glibc's chunk2mem.  The user pointer IS the entry address.
        return static_cast<void*>(e);
    }

    // Free to tcache (lock-free fast path)
    // `p` is a Chunk* (chunk start).  TcacheEntry lives at p->user_data(),
    // i.e. chunk + CHUNK_HDR_SZ, matching glibc's chunk2mem(p).
    // Returns false if tcache bin is full.
    bool free(TcacheIdx idx, Chunk* p) noexcept {
        if (idx.value >= TCACHE_MAX_BINS) return false;
        if (counts_[idx.value] >= TCACHE_FILL_COUNT) return false;

        // Place TcacheEntry at user data location (preserves chunk header)
        TcacheEntry* e = reinterpret_cast<TcacheEntry*>(p->user_data());
        e->key = reinterpret_cast<TcachePerthread*>(random_key_);  // Double-free detection
        e->next = protect_ptr(e, entries_[idx.value]);              // Safe-linking
        entries_[idx.value] = e;
        counts_[idx.value]++;
        return true;
    }

    // Check if a chunk looks like a double-free (key matches)
    [[nodiscard]] bool is_double_free(TcacheEntry* e) const noexcept {
        return e->key == reinterpret_cast<TcachePerthread*>(random_key_);
    }

    // Flush all tcache bins back to arena (called on thread exit)
    void flush_all(class Arena& arena) noexcept;

    // Safe-linking: mangle pointer with position-based XOR
    static TcacheEntry* protect_ptr(TcacheEntry* pos, TcacheEntry* ptr) noexcept {
        return reinterpret_cast<TcacheEntry*>(
            (reinterpret_cast<uintptr_t>(pos) >> 12) ^
            reinterpret_cast<uintptr_t>(ptr));
    }

    // Safe-linking: reveal mangled pointer (XOR is self-inverse)
    static TcacheEntry* reveal_ptr(TcacheEntry* pos, TcacheEntry* protected_ptr) noexcept {
        return protect_ptr(pos, protected_ptr);
    }
};

// Thread-local tcache pointer
inline thread_local TcachePerthread* tcache = nullptr;

// Initialize tcache for current thread
void tcache_init() noexcept;

// Shutdown tcache for current thread (flush to arena)
void tcache_shutdown(class Arena& arena) noexcept;

} // namespace my_ptmalloc
