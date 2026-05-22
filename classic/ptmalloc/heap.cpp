// HeapInfo implementation — mmap'd heap segment management.

#include "heap.h"
#include "sys_memory.h"

#include <cstring>
#include <atomic>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Global heap registry ───
HeapInfo* HeapRegistry::head_ = nullptr;

// ─── Global arena list ───
// All arenas are tracked in a linked list starting at main_arena.
// This is initialized by ArenaManager.
extern Arena* g_all_arenas_head;

void HeapRegistry::register_heap(HeapInfo* h) noexcept {
    h->prev = head_;
    head_ = h;
}

Arena* HeapRegistry::find_arena(Chunk* p) noexcept {
    // Walk the global HeapInfo chain
    for (HeapInfo* h = head_; h; h = h->prev) {
        if (h->contains_chunk(p)) {
            return h->ar_ptr;
        }
    }
    return nullptr;
}

// ─── HeapInfo ───
HeapInfo* HeapInfo::new_heap(size_t size, Arena* ar, SysMemory* sysmem) noexcept {
    // Allocate a heap segment big enough for HeapInfo header + requested size
    size_t total = sizeof(HeapInfo) + size;
    if (total < HEAP_MIN_SIZE) total = HEAP_MIN_SIZE;
    total = (total + SysMemory::page_size() - 1) & ~(SysMemory::page_size() - 1);

    void* raw = sysmem->map(total);
    if (!raw) return nullptr;

    auto* h = static_cast<HeapInfo*>(raw);
    h->ar_ptr = ar;
    h->prev = nullptr;
    h->size = total;
    h->mprotect_size = 0;
    std::memset(h->pad, 0, sizeof(h->pad));

    HeapRegistry::register_heap(h);
    return h;
}

Arena* HeapInfo::arena_for_chunk(Chunk* p) noexcept {
    return HeapRegistry::find_arena(p);
}

} // namespace ptmalloc
} // namespace my_ptmalloc
