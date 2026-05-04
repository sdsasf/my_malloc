#pragma once
// HeapInfo: descriptor for non-main arena heap regions
// Non-main arenas are backed by mmap'd regions, each starting with HeapInfo
// Aligned to HEAP_MAX_SIZE so heap_for_ptr() can mask off low bits

#include "config.h"
#include <cstddef>

namespace my_ptmalloc {

struct Arena;  // Forward declaration

struct HeapInfo {
    Arena*    ar_ptr;         // Pointer to owning arena
    HeapInfo* prev;           // Previous heap for this arena (linked list)
    size_t    size;           // Total size of this heap region
    size_t    mprotect_size;  // Bytes mprotected
    size_t    pagesize;       // Page size at allocation time

    // Padding to ensure proper alignment
    // (sizeof(HeapInfo) + CHUNK_HDR_SZ) must be MALLOC_ALIGNMENT-aligned
    char pad_[MALLOC_ALIGNMENT - (sizeof(Arena*) * 2 + sizeof(size_t) * 3) % MALLOC_ALIGNMENT];

    // Find the heap header for a given pointer
    // Works because heaps are HEAP_MAX_SIZE-aligned
    [[nodiscard]] static HeapInfo* heap_for_ptr(void* p) noexcept {
        return reinterpret_cast<HeapInfo*>(
            reinterpret_cast<uintptr_t>(p) & ~(HEAP_MAX_SIZE - 1));
    }

    // Get the arena that owns a given chunk
    [[nodiscard]] static Arena* arena_for_chunk(void* p, bool main_arena) noexcept;

    // Allocate a new heap region, linked to the given arena
    [[nodiscard]] static HeapInfo* new_heap(size_t size, Arena* arena, HeapInfo* prev) noexcept;
};

} // namespace my_ptmalloc
