#pragma once
// Chunk: the fundamental memory unit
// Each allocated/free region starts with a Chunk header
// Matches glibc malloc_chunk layout exactly

#include "config.h"
#include "types.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace my_ptmalloc {

// Forward declarations
struct Arena;

struct Chunk {
    size_t prev_size;    // Size of previous contiguous chunk (valid if prev is free)
    size_t size;         // Size of THIS chunk; low 3 bits = flags (P|M|A)
    Chunk* fd;           // Forward pointer (valid only when FREE)
    Chunk* bk;           // Backward pointer (valid only when FREE)
    Chunk* fd_nextsize;  // Large bin: next larger chunk (FREE, large only)
    Chunk* bk_nextsize;  // Large bin: prev smaller chunk (FREE, large only)

    // ─── Size and flag accessors ───

    [[nodiscard]] ChunkSize chunk_size() const noexcept {
        return ChunkSize{size & ~SIZE_BITS};
    }

    [[nodiscard]] bool prev_inuse() const noexcept {
        return (size & PREV_INUSE_BIT) != 0;
    }

    [[nodiscard]] bool is_mmapped() const noexcept {
        return (size & IS_MMAPPED_BIT) != 0;
    }

    [[nodiscard]] bool is_main_arena() const noexcept {
        return (size & NON_MAIN_ARENA_BIT) == 0;
    }

    [[nodiscard]] ChunkFlag flags() const noexcept {
        return static_cast<ChunkFlag>(size & SIZE_BITS);
    }

    // ─── Navigation ───

    [[nodiscard]] Chunk* next_chunk() const noexcept {
        return reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(this) + chunk_size().value);
    }

    [[nodiscard]] Chunk* prev_chunk() const noexcept {
        return reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(this) - prev_size);
    }

    // ─── Pointer conversion ───

    // In ptmalloc, user data starts at chunk + 2*SIZE_SZ (16 bytes on 64-bit)
    // Both prev_size and size fields are hidden from the user
    [[nodiscard]] void* user_data() noexcept {
        return reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(this) + CHUNK_HDR_SZ);
    }

    [[nodiscard]] static Chunk* from_user_ptr(void* p) noexcept {
        return reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(p) - CHUNK_HDR_SZ);
    }

    // ─── Size manipulation ───

    void set_size(ChunkSize cs) noexcept {
        // Preserve flag bits, replace size
        size = cs.value | (size & SIZE_BITS);
    }

    void set_flags(ChunkFlag f) noexcept {
        size = (size & ~SIZE_BITS) | static_cast<size_t>(f);
    }

    void set_head(ChunkSize cs, ChunkFlag f) noexcept {
        size = cs.value | static_cast<size_t>(f);
    }

    void set_foot(ChunkSize cs) noexcept {
        // Write prev_size into the next chunk's prev_size field
        next_chunk()->prev_size = cs.value;
    }

    void clear_previnuse() noexcept {
        next_chunk()->size &= ~PREV_INUSE_BIT;
    }

    void set_previnuse() noexcept {
        next_chunk()->size |= PREV_INUSE_BIT;
    }

    // Mark chunk as allocated: set PREV_INUSE on next chunk
    void mark_inuse() noexcept {
        size_t cs = chunk_size().value;
        if (cs == 0) {
#if MY_PTMALLOC_DEBUG
            // If size is 0, next_chunk() returns self, corrupting our own size
            // Skip to avoid self-corruption
            fprintf(stderr, "[mark_inuse] SKIP zero-size chunk=%p raw=0x%zx\n",
                    (void*)this, size);
#endif
            return;
        }
        next_chunk()->size |= PREV_INUSE_BIT;
    }

    // ─── Validation ───

    [[nodiscard]] bool is_valid() const noexcept {
        return chunk_size().value >= MINSIZE &&
               (chunk_size().value & MALLOC_ALIGN_MASK) == 0;
    }
};

static_assert(sizeof(Chunk) == 6 * SIZE_SZ,
    "Chunk must be exactly 6 * SIZE_SZ bytes (48 on 64-bit)");

static_assert(offsetof(Chunk, prev_size) == 0);
static_assert(offsetof(Chunk, size) == SIZE_SZ);
static_assert(offsetof(Chunk, fd) == 2 * SIZE_SZ);
static_assert(offsetof(Chunk, bk) == 3 * SIZE_SZ);

} // namespace my_ptmalloc
