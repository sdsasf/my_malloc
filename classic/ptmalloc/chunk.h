#pragma once
// ptmalloc boundary-tag chunk — faithful to glibc's malloc_chunk.

#include "config.h"

#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Chunk structure ───
// Layout (when in-use):
//   [prev_size (8B)] [size|flags (8B)] [user data ...]
// Layout (when free):
//   [prev_size (8B)] [size|flags (8B)] [fd (8B)] [bk (8B)] [fd_nextsize (8B)] [bk_nextsize (8B)] ...
//
// Key insight (boundary-tag consolidation):
//   The prev_size field of a chunk, when the PREV_INUSE bit of the NEXT chunk
//   is clear, stores the size of the previous (free) chunk. This allows
//   O(1) backward coalescing: prev = p - p->prev_size.

struct Chunk {
    size_t prev_size;   // Size of previous chunk (valid only if prev is free)
    size_t size;        // Size + flags (low 3 bits: P|M|A)

    // ─── When free: forward/backward pointers in circular doubly-linked lists ───
    // These overlay the user data area. For largebin chunks, fd_nextsize/bk_nextsize
    // form a second doubly-linked list ordered by size (no exact-fit duplicates).

    Chunk* fd;           // Forward pointer (free list)
    Chunk* bk;           // Backward pointer (free list)
    // For largebins only: pointers to next/prev chunk of DIFFERENT size
    Chunk* fd_nextsize;
    Chunk* bk_nextsize;

    // ─── Accessors ───

    size_t chunk_size() const noexcept {
        return size & ~(PREV_INUSE_BIT | IS_MMAPPED_BIT | NON_MAIN_ARENA_BIT);
    }

    ChunkFlag flags() const noexcept {
        return static_cast<ChunkFlag>(size & (PREV_INUSE_BIT | IS_MMAPPED_BIT | NON_MAIN_ARENA_BIT));
    }

    bool prev_inuse() const noexcept { return size & PREV_INUSE_BIT; }
    bool is_mmapped()  const noexcept { return size & IS_MMAPPED_BIT; }
    bool is_main_arena() const noexcept { return !(size & NON_MAIN_ARENA_BIT); }

    // ─── Mutators ───

    void set_head(ChunkSize sz, ChunkFlag flags) noexcept {
        size = sz.value | static_cast<size_t>(flags);
    }
    void set_head_from(Chunk* other) noexcept {
        size = other->size;
    }
    void set_foot(ChunkSize sz) noexcept {
        // Foot = prev_size field of the *next* contiguous chunk
        Chunk* next_chunk = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(this) + sz.value);
        next_chunk->prev_size = sz.value;
    }

    void mark_inuse() noexcept {
        // Set PREV_INUSE on the NEXT contiguous chunk
        Chunk* next_chunk = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(this) + chunk_size());
        next_chunk->size |= PREV_INUSE_BIT;
    }

    void clear_previnuse() noexcept {
        // Clear PREV_INUSE on the NEXT contiguous chunk
        Chunk* next_chunk = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(this) + chunk_size());
        next_chunk->size &= ~PREV_INUSE_BIT;
    }

    // ─── Navigation ───

    Chunk* next_chunk() const noexcept {
        return reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(this) + chunk_size());
    }

    void* user_data() noexcept {
        return reinterpret_cast<void*>(this + 1);  // right after chunk header
    }

    static Chunk* from_user_ptr(void* ptr) noexcept {
        return static_cast<Chunk*>(ptr) - 1;
    }
};

// ─── Size computation ───

// Convert user request to chunk size (rounded up, minimum MINSIZE)
inline ChunkSize request2size(UserSize req) noexcept {
    size_t nb = req.value;
    if (nb + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE) {
        return ChunkSize{MINSIZE};
    }
    size_t aligned = (nb + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;
    if (aligned < MINSIZE) aligned = MINSIZE;
    return ChunkSize{aligned};
}

// ─── Bin indexing ───

// Fastbin index from chunk size
inline FastbinIdx fastbin_index(ChunkSize sz) noexcept {
    size_t s = sz.value;
    // On 64-bit: fastbins cover chunk sizes 32, 48, 64, 80, 96, 112, 128, 144, 160
    // Index = (size >> 4) - 2
    unsigned idx = static_cast<unsigned>((s >> 4) - 2);
    return FastbinIdx{idx < NFASTBINS ? idx : NFASTBINS};
}

// Smallbin index from chunk size (glibc: smallbin_index)
inline SmallbinIdx smallbin_index(ChunkSize sz) noexcept {
    // Smallbins cover chunk sizes 32, 48, 64, ..., 1008 (MINSIZE to MIN_LARGE_SIZE-16)
    // bin index = size / 16  (offset into bin array)
    unsigned idx = static_cast<unsigned>(sz.value >> 4);
    return SmallbinIdx{idx};
}

// Tcache index from chunk size
inline TcacheIdx csize2tidx(ChunkSize sz) noexcept {
    size_t s = sz.value;
    // tcace bin index = (chunk_size / MALLOC_ALIGNMENT) - 2
    // Covers chunk sizes from MINSIZE(32) to TCACHE_MAX_BINS*16+16
    unsigned idx = static_cast<unsigned>((s >> 4) - 2);
    return TcacheIdx{idx < TCACHE_MAX_BINS ? idx : TCACHE_MAX_BINS};
}

// Does this chunk size fall in tcache range?
inline bool in_tcache_range(ChunkSize sz) noexcept {
    size_t s = sz.value;
    return s <= (TCACHE_MAX_BINS + 1) * MALLOC_ALIGNMENT;
}

// Does this chunk size fall in smallbin range?
inline bool in_smallbin_range(ChunkSize sz) noexcept {
    return sz.value < MIN_LARGE_SIZE;
}

// ─── Logging helpers ───
#ifdef PTMALLOC_DEBUG
#include <cstdio>
#define PTMALLOC_LOG(fmt, ...) std::fprintf(stderr, "[ptmalloc] " fmt "\n", ##__VA_ARGS__)
#else
#define PTMALLOC_LOG(...) ((void)0)
#endif

} // namespace ptmalloc
} // namespace my_ptmalloc
