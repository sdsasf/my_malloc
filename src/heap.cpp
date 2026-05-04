// Heap allocation for non-main arenas

#include "my_ptmalloc/heap.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/arena_manager.h"

#include <sys/mman.h>
#include <cstring>
#include <unistd.h>

namespace my_ptmalloc {

Arena* HeapInfo::arena_for_chunk(void* p, bool is_main) noexcept {
    if (is_main) return g_arena_manager ? g_arena_manager->get_main_arena() : nullptr;
    return heap_for_ptr(p)->ar_ptr;
}

HeapInfo* HeapInfo::new_heap(size_t requested_size, Arena* arena, HeapInfo* prev) noexcept {
    // Round up to HEAP_MAX_SIZE alignment
    size_t size = (requested_size + HEAP_MAX_SIZE - 1) & ~(HEAP_MAX_SIZE - 1);
    if (size < HEAP_MAX_SIZE) size = HEAP_MAX_SIZE;

    size_t map_size = size + HEAP_MAX_SIZE;
    void* raw = ::mmap(nullptr, map_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) return nullptr;

    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t aligned = (raw_addr + HEAP_MAX_SIZE - 1) & ~(HEAP_MAX_SIZE - 1);
    size_t prefix = aligned - raw_addr;
    size_t suffix = (raw_addr + map_size) - (aligned + size);
    if (prefix) ::munmap(reinterpret_cast<void*>(raw_addr), prefix);
    if (suffix) ::munmap(reinterpret_cast<void*>(aligned + size), suffix);

    HeapInfo* h = reinterpret_cast<HeapInfo*>(aligned);
    h->ar_ptr = arena;
    h->prev = prev;
    h->size = size;
    h->pagesize = 4096;  // Default page size

    return h;
}

} // namespace my_ptmalloc
