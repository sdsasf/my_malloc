#pragma once
// HeapInfo: mmap'd heap segment management for non-main arenas.
// Each non-main arena has a linked list of HeapInfo structs
// describing its mmap'd heap regions.

#include "config.h"
#include "chunk.h"
#include "arena.h"

namespace my_ptmalloc {
namespace ptmalloc {

struct SysMemory;

// ─── HeapInfo ───
// Placed at the start of each mmap'd heap segment for a non-main arena.
// Forms a singly-linked list so we can find which arena owns a given chunk.

struct HeapInfo {
    Arena*   ar_ptr;      // Arena that owns this heap
    HeapInfo* prev;        // Previous heap in chain
    size_t   size;         // Size of this heap segment
    size_t   mprotect_size; // Padding (not used in simplified version)
    char     pad[16];      // Pad to align first chunk

    // Allocate a new heap segment for an arena
    static HeapInfo* new_heap(size_t size, Arena* ar, SysMemory* sysmem) noexcept;

    // Find the arena that owns a given chunk pointer
    // Iterates the HeapInfo chain of all non-main arenas
    static Arena* arena_for_chunk(Chunk* p) noexcept;

    // Check if a chunk belongs to this heap
    bool contains_chunk(Chunk* p) const noexcept {
        uintptr_t heap_start = reinterpret_cast<uintptr_t>(this);
        uintptr_t heap_end   = heap_start + size;
        uintptr_t chunk_addr = reinterpret_cast<uintptr_t>(p);
        return chunk_addr >= heap_start && chunk_addr < heap_end;
    }
};

// ─── Global heap registry ───
// All HeapInfo structures are tracked so we can find any chunk's arena.
// In glibc this is through the main_arena.next chain of arenas,
// each with its own HeapInfo list.

struct HeapRegistry {
    static HeapInfo* head_;

    static void register_heap(HeapInfo* h) noexcept;
    static Arena* find_arena(Chunk* p) noexcept;
};

} // namespace ptmalloc
} // namespace my_ptmalloc
