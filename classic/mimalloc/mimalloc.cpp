// mimalloc core implementation.
// Design: per-thread heaps, segment→page→block hierarchy,
// pointer-arithmetic metadata lookup, atomic remote-free queues.

#include "mimalloc.h"
#include "segment.h"
#include "os.h"

#include <pthread.h>
#include <atomic>
#include <cstring>
#include <new>

namespace my_ptmalloc {
namespace mimalloc {

// ─── Globals ───
thread_local mi_heap_t* mi_thread_heap = nullptr;
static pthread_once_t    mi_init_once = PTHREAD_ONCE_INIT;

mi_abandoned_t mi_abandoned;

// Global heap list head
static std::atomic<mi_heap_t*> mi_heap_list{nullptr};
static std::atomic<uint32_t>   mi_next_heap_id{1};

// ─── Page helpers ───

uintptr_t mi_page_t::block_start() const noexcept {
    // Blocks start right after the page header within the segment
    uintptr_t page_addr = reinterpret_cast<uintptr_t>(this);
    mi_segment_t* seg = reinterpret_cast<mi_segment_t*>(page_addr & ~MI_SEGMENT_MASK);
    // The page's blocks are in the segment's data area
    // For simplicity: block area starts at segment + page_offset
    // Real mimalloc uses a more precise layout.
    uintptr_t seg_data = reinterpret_cast<uintptr_t>(seg)
                       + sizeof(mi_segment_t)
                       + seg->page_capacity * sizeof(mi_page_t);
    // Align to block alignment
    seg_data = (seg_data + MI_BLOCK_ALIGN - 1) & ~static_cast<uintptr_t>(MI_BLOCK_ALIGN - 1);

    // Each page gets an equal slice of the data area
    return seg_data + segment_idx * (MI_SEGMENT_SIZE / seg->page_capacity - sizeof(mi_page_t));
}

uintptr_t mi_page_t::block_at(size_t i) const noexcept {
    return block_start() + i * block_size;
}

size_t mi_page_t::block_index(mi_block_t* block) const noexcept {
    uintptr_t b = reinterpret_cast<uintptr_t>(block);
    uintptr_t s = block_start();
    return (b - s) / block_size;
}

void mi_page_t::reset() noexcept {
    // Return all blocks to local_free list
    local_free = nullptr;
    local_count = 0;
    for (size_t i = 0; i < capacity; i++) {
        auto* block = reinterpret_cast<mi_block_t*>(block_at(i));
        // Encode pointer
        block->next = local_free;
        local_free = block;
        local_count++;
    }
    used = 0;
    thread_free = nullptr;
    thread_free_count = 0;
    xthread_free.store(nullptr, std::memory_order_release);
}

size_t mi_page_t::drain_remote() noexcept {
    // Atomically take all entries from xthread_free queue
    mi_block_t* list = xthread_free.exchange(nullptr, std::memory_order_acquire);
    if (!list) return 0;

    // Reverse the LIFO queue into FIFO order for fairness
    mi_block_t* reversed = nullptr;
    size_t count = 0;
    while (list) {
        mi_block_t* next = list->next;
        list->next = reversed;
        reversed = list;
        list = next;
        count++;
    }

    // Prepend to local_free
    if (reversed) {
        mi_block_t* tail = reversed;
        while (tail->next) tail = tail->next;
        tail->next = local_free;
        local_free = reversed;
        local_count += static_cast<uint32_t>(count);
    }
    return count;
}

size_t mi_page_t::drain_thread_free() noexcept {
    if (!thread_free) return 0;
    // Prepend thread_free to local_free
    mi_block_t* tail = thread_free;
    size_t count = 1;
    while (tail->next) { tail = tail->next; count++; }
    tail->next = local_free;
    local_free = thread_free;
    local_count += static_cast<uint32_t>(count);
    thread_free = nullptr;
    thread_free_count = 0;
    return count;
}

// ─── Abandoned page management ───

void mi_abandoned_t::push(mi_page_t* page) noexcept {
    mi_page_t* old = head.load(std::memory_order_acquire);
    do {
        page->next = old;
    } while (!head.compare_exchange_weak(old, page,
             std::memory_order_release, std::memory_order_acquire));
}

mi_page_t* mi_abandoned_t::pop() noexcept {
    mi_page_t* page = head.load(std::memory_order_acquire);
    while (page) {
        mi_page_t* next = page->next;
        if (head.compare_exchange_weak(page, next,
                std::memory_order_release, std::memory_order_acquire)) {
            return page;
        }
    }
    return nullptr;
}

// ─── Heap management ───

mi_heap_t* mi_heap_t::create() noexcept {
    void* mem = OS::raw_alloc(sizeof(mi_heap_t));
    if (!mem) return nullptr;

    auto* heap = new (mem) mi_heap_t();
    heap->heap_id = mi_next_heap_id.fetch_add(1, std::memory_order_relaxed);
    // Randomize cookie (simplified: use heap_id)
    heap->cookie = (static_cast<uint32_t>(heap->heap_id) * 2654435761u) | 1;

    heap->pages = nullptr;
    heap->pages_free = nullptr;
    heap->pages_full = nullptr;
    heap->delayed_free = nullptr;
    heap->delayed_count = 0;
    heap->allocated = 0;
    heap->peak = 0;

    for (size_t i = 0; i < MI_CLASS_COUNT; i++) {
        heap->page_direct[i] = nullptr;
    }

    // Add to global heap list
    heap->prev = nullptr;
    mi_heap_t* old = mi_heap_list.load(std::memory_order_acquire);
    do {
        heap->next = old;
    } while (!mi_heap_list.compare_exchange_weak(old, heap,
             std::memory_order_release, std::memory_order_acquire));

    return heap;
}

void mi_heap_t::destroy() noexcept {
    // Abandon all pages (push to abandoned list for other threads to reclaim)
    mi_page_t* page = pages;
    while (page) {
        mi_page_t* next = page->next;
        page->heap_tag = 0;  // mark as abandoned
        mi_abandoned.push(page);
        page = next;
    }
}

mi_heap_t* mi_heap_t::get() noexcept {
    mi_thread_init();
    return mi_thread_heap;
}

// ─── Thread initialization ───

void mi_thread_init() noexcept {
    if (mi_thread_heap) return;
    mi_thread_heap = mi_heap_t::create();
}

// ─── Page allocation ───

static mi_page_t* mi_page_alloc(mi_heap_t* heap, size_t class_idx) noexcept {
    size_t block_sz = mi_class_size(class_idx);

    // Determine how many blocks fit in a page
    size_t page_payload = MI_SEGMENT_SIZE - sizeof(mi_segment_t) - sizeof(mi_page_t);
    size_t capacity = page_payload / block_sz;
    if (capacity > 65535) capacity = 65535;  // practical limit

    mi_segment_t* seg = mi_segment_alloc(1);
    if (!seg) return nullptr;

    mi_page_t* page = &seg->pages[0];
    page->block_size = static_cast<uint32_t>(block_sz);
    page->capacity = static_cast<uint32_t>(capacity);
    page->used = 0;
    page->reserved = 0;
    page->local_free = nullptr;
    page->local_count = 0;
    page->thread_free = nullptr;
    page->thread_free_count = 0;
    page->xthread_free.store(nullptr, std::memory_order_release);
    page->heap_tag = heap->heap_id;
    page->segment_idx = 0;
    page->next = nullptr;
    page->prev = nullptr;

    // Generate cookie for pointer encoding
    page->cookie = (reinterpret_cast<uintptr_t>(page) >> 4) | 1;

    seg->page_count = 1;
    seg->commit_mask = 1;

    // Initialize all blocks as free
    for (size_t i = 0; i < capacity; i++) {
        auto* block = reinterpret_cast<mi_block_t*>(
            reinterpret_cast<uintptr_t>(seg)
            + sizeof(mi_segment_t)
            + sizeof(mi_page_t)
            + i * block_sz);
        // Encode next pointer with cookie
        block->next = reinterpret_cast<mi_block_t*>(
            reinterpret_cast<uintptr_t>(page->local_free) ^ page->cookie);
        page->local_free = block;
        page->local_count++;
    }

    // Add to heap page list
    page->next = heap->pages;
    if (heap->pages) heap->pages->prev = page;
    heap->pages = page;

    // Set as direct page for this size class
    heap->page_direct[class_idx] = page;

    return page;
}

// ─── Allocation ───

void* mi_malloc(size_t size) noexcept {
    mi_init();
    if (size == 0) size = 1;

    // Large allocation: direct OS
    if (size > MI_MAX_SMALL_SIZE) {
        size_t actual = (size + sizeof(size_t) + 15) & ~15;
        void* raw = OS::raw_alloc(actual + sizeof(size_t));
        if (!raw) return nullptr;
        // Store size before user data
        *static_cast<size_t*>(raw) = actual;
        void* user = static_cast<char*>(raw) + sizeof(size_t);
        mi_heap_t* heap = mi_heap_t::get();
        heap->allocated += size;
        if (heap->allocated > heap->peak) heap->peak = heap->allocated;
        return user;
    }

    mi_heap_t* heap = mi_heap_t::get();
    size_t class_idx = mi_class_index(size);

    // Try per-size-class direct page first
    mi_page_t* page = heap->page_direct[class_idx];
    if (!page || page->local_free == nullptr) {
        // Try reclaiming abandoned page
        page = mi_abandoned.pop();
        if (page && page->block_size == mi_class_size(class_idx)) {
            page->heap_tag = heap->heap_id;
            page->next = heap->pages;
            if (heap->pages) heap->pages->prev = page;
            heap->pages = page;
            heap->page_direct[class_idx] = page;
        } else {
            // Allocate new page
            page = mi_page_alloc(heap, class_idx);
            if (!page) return nullptr;
        }
    }

    // Drain remote frees
    if (page->xthread_free.load(std::memory_order_acquire)) {
        page->drain_remote();
    }
    // Drain thread_free
    if (page->thread_free) {
        page->drain_thread_free();
    }

    // Pop from local_free
    if (!page->local_free) {
        // No free blocks — need new page
        page = mi_page_alloc(heap, class_idx);
        if (!page) return nullptr;
    }

    // Decode pointer
    mi_block_t* block = page->local_free;
    page->local_free = reinterpret_cast<mi_block_t*>(
        reinterpret_cast<uintptr_t>(block->next) ^ page->cookie);
    page->local_count--;
    page->used++;
    block->next = nullptr;

    // Move page between lists if necessary
    if (page->used == page->capacity) {
        // Page became full — remove from direct
        heap->page_direct[class_idx] = nullptr;
    }

    heap->allocated += page->block_size;
    if (heap->allocated > heap->peak) heap->peak = heap->allocated;

    return static_cast<void*>(block);
}

// ─── Free ───

void mi_free(void* ptr) noexcept {
    if (!ptr) return;

    // Large allocation: direct OS free
    // Check if ptr is in a segment
    mi_segment_t* seg = mi_segment_t::from_ptr(ptr);
    if (!seg || seg->magic != 0x6d696d616c6c6f63ULL) {
        // Treat as large (direct OS) allocation
        void* raw = static_cast<char*>(ptr) - sizeof(size_t);
        size_t actual = *static_cast<size_t*>(raw);
        OS::raw_free(raw, actual + sizeof(size_t));
        return;
    }

    mi_page_t* page = mi_segment_page_of(ptr);
    if (!page) return;

    mi_heap_t* heap = mi_thread_heap;
    auto* block = static_cast<mi_block_t*>(ptr);

    if (heap && page->heap_tag == heap->heap_id) {
        // Same thread — fast path: push to local_free
        block->next = reinterpret_cast<mi_block_t*>(
            reinterpret_cast<uintptr_t>(page->local_free) ^ page->cookie);
        page->local_free = block;
        page->local_count++;
        page->used--;

        if (heap) heap->allocated -= page->block_size;
    } else {
        // Cross-thread free — use atomic CAS on xthread_free
        mi_block_t* old = page->xthread_free.load(std::memory_order_relaxed);
        do {
            block->next = reinterpret_cast<mi_block_t*>(
                reinterpret_cast<uintptr_t>(old) ^ page->cookie);
        } while (!page->xthread_free.compare_exchange_weak(
            old, block, std::memory_order_release, std::memory_order_relaxed));
        page->used--;
    }
}

// ─── Calloc ───

void* mi_calloc(size_t n, size_t size) noexcept {
    size_t total = n * size;
    if (n != 0 && total / n != size) return nullptr;
    void* p = mi_malloc(total);
    if (p) {
        size_t usable = mi_usable_size(p);
        std::memset(p, 0, usable < total ? usable : total);
    }
    return p;
}

// ─── Realloc ───

void* mi_realloc(void* ptr, size_t size) noexcept {
    if (!ptr) return mi_malloc(size);
    if (size == 0) { mi_free(ptr); return nullptr; }

    size_t old_size = mi_usable_size(ptr);
    if (old_size == 0) {
        // Large allocation — realloc with OS
        void* raw = static_cast<char*>(ptr) - sizeof(size_t);
        size_t actual = *static_cast<size_t*>(raw);
        if (size <= actual) return ptr;
        void* np = mi_malloc(size);
        if (!np) return nullptr;
        std::memcpy(np, ptr, actual);
        mi_free(ptr);
        return np;
    }
    if (size <= old_size) return ptr;

    void* np = mi_malloc(size);
    if (!np) return nullptr;
    std::memcpy(np, ptr, old_size);
    mi_free(ptr);
    return np;
}

// ─── Memalign ───

void* mi_memalign(size_t alignment, size_t size) noexcept {
    if (alignment <= MI_BLOCK_ALIGN) return mi_malloc(size);
    if ((alignment & (alignment - 1)) != 0) return nullptr;

    // For large alignments, use OS directly
    size_t actual = (size + sizeof(size_t) + alignment - 1) & ~(alignment - 1);
    void* raw = OS::raw_alloc(actual + sizeof(size_t) + alignment);
    if (!raw) return nullptr;

    *static_cast<size_t*>(raw) = actual;
    uintptr_t user = reinterpret_cast<uintptr_t>(raw) + sizeof(size_t);
    user = (user + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<void*>(user);
}

// ─── Usable size ───

size_t mi_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;

    mi_segment_t* seg = mi_segment_t::from_ptr(ptr);
    if (!seg || seg->magic != 0x6d696d616c6c6f63ULL) {
        // Large allocation
        void* raw = static_cast<char*>(ptr) - sizeof(size_t);
        return *static_cast<size_t*>(raw);
    }

    mi_page_t* page = mi_segment_page_of(ptr);
    if (!page) return 0;
    return page->block_size;
}

// ─── Init ───

void mi_init() noexcept {
    pthread_once(&mi_init_once, []() {
        mi_thread_init();
    });
}

} // namespace mimalloc
} // namespace my_ptmalloc
