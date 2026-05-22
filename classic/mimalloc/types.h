#pragma once
// mimalloc core types: heap, segment, page, block.

#include <cstddef>
#include <cstdint>
#include <atomic>

namespace my_ptmalloc {
namespace mimalloc {

// ─── Constants ───
constexpr size_t MI_SEGMENT_SIZE       = 64 * 1024;  // 64KB segments
constexpr size_t MI_SEGMENT_MASK       = MI_SEGMENT_SIZE - 1;
constexpr size_t MI_SEGMENT_ALIGN      = MI_SEGMENT_SIZE;

constexpr size_t MI_MAX_SMALL_SIZE     = 1024 * 8;   // 8KB – larger = direct OS
constexpr size_t MI_PAGE_MEDIUM_SIZE   = 1024 * 64;  // 64KB for medium

// Page states
constexpr size_t MI_SMALL_PAGE_SIZE    = 64 * 1024;  // small page = 1 segment

constexpr size_t MI_BLOCK_ALIGN        = 16;         // minimum alignment
constexpr size_t MI_MAX_ALIGN          = 1024 * 1024; // 1MB max alignment

// Size class configuration: 8, 16, 24, ..., up to MI_MAX_SMALL_SIZE
// Actually, mimalloc uses many more size classes. For learning, we use
// 16-byte steps up to 4KB, then varied steps up to 8KB.
constexpr size_t MI_CLASS_STEP_SMALL   = 16;
constexpr size_t MI_CLASS_COUNT_SMALL  = 256;  // 16 * 256 = 4096 (up to 4KB in 16B steps)
constexpr size_t MI_CLASS_COUNT_MEDIUM = 32;   // 4KB+ in steps of 128B up to 8KB
constexpr size_t MI_CLASS_COUNT        = MI_CLASS_COUNT_SMALL + MI_CLASS_COUNT_MEDIUM;

// ─── Type aliases ───
using mi_tagged_ptr_t = uintptr_t;

// ─── Free list block (overlays user data) ───
// Encoded: free list pointers are XOR'd with a per-page cookie to
// prevent exploitation of freelist corruption.
struct mi_block_t {
    mi_block_t* next;  // encoded: next ^ cookie
};

// ─── Page ───
// Each page belongs to a specific size class. Pages are organized
// within a heap's page list.
struct mi_page_t {
    // ─── Block metadata ───
    uint32_t    block_size;       // size of each block in this page
    uint32_t    capacity;         // max number of blocks
    uint32_t    used;             // number of blocks in use
    uint32_t    reserved;         // reservation count (for deferred free)

    // ─── Free lists ───
    mi_block_t* local_free;       // fast-path: local free list (LIFO)
    uint32_t    local_count;      // number of blocks on local_free
    mi_block_t* thread_free;      // deferred free from same thread
    uint32_t    thread_free_count;

    // ─── Cross-thread free queue (atomic) ───
    std::atomic<mi_block_t*> xthread_free;  // CAS-based remote-free queue

    // ─── Page metadata ───
    uint32_t    heap_tag;         // owner heap identifier
    uintptr_t   cookie;           // encoding cookie for free list obfuscation
    uint32_t    segment_idx;      // index within segment's page array

    // ─── Page list pointers ───
    mi_page_t*  next;             // next page in heap's page list
    mi_page_t*  prev;             // prev page in heap's page list

    // ─── Block management ───
    uintptr_t   block_start() const noexcept;
    uintptr_t   block_at(size_t i) const noexcept;
    size_t      block_index(mi_block_t* block) const noexcept;

    // Reset page (return all blocks to free state)
    void reset() noexcept;
    // Drain xthread_free queue into local_free
    size_t drain_remote() noexcept;
    // Drain thread_free into local_free
    size_t drain_thread_free() noexcept;
};

// ─── Segment ───
// A segment is a 64KB-aligned memory region containing pages.
// Metadata is at the start, pages follow.
struct mi_segment_t {
    uint64_t    magic;            // 0x6d696d616c6c6f63 "mimalloc" in little-endian
    mi_segment_t* next;          // next in segment cache
    uint32_t    page_count;       // number of pages in this segment
    uint32_t    page_capacity;    // max pages
    uint32_t    abandoned;        // is this segment abandoned?
    uint32_t    commit_mask;      // which pages are committed

    // Page array follows the segment header. Each page is at offset
    // page_size * i from the segment base (after metadata).
    mi_page_t   pages[];          // flexible array member

    // Given a pointer, find its segment (aligned to MI_SEGMENT_SIZE)
    static mi_segment_t* from_ptr(void* p) noexcept {
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        return reinterpret_cast<mi_segment_t*>(addr & ~MI_SEGMENT_MASK);
    }

    // Given a pointer, find its page
    static mi_page_t* page_of(void* p) noexcept {
        mi_segment_t* seg = from_ptr(p);
        if (!seg || seg->magic != 0x6d696d616c6c6f63ULL) return nullptr;
        // The page is at a known index based on block offset within segment
        // Simplified: linear scan (real mimalloc uses fast division)
        uintptr_t offset = reinterpret_cast<uintptr_t>(p) - reinterpret_cast<uintptr_t>(seg);
        uintptr_t page_offset = offset / MI_SEGMENT_SIZE;  // approximation
        // For learning: walk pages to find the one containing this ptr
        for (uint32_t i = 0; i < seg->page_count; i++) {
            mi_page_t* page = &seg->pages[i];
            if (page->block_size == 0) continue;
            uintptr_t bstart = page->block_start();
            uintptr_t bend   = bstart + page->capacity * page->block_size;
            if (reinterpret_cast<uintptr_t>(p) >= bstart &&
                reinterpret_cast<uintptr_t>(p) < bend) {
                return page;
            }
        }
        return nullptr;
    }
};

// ─── Heap ───
// Each thread gets its own heap. The heap owns a list of pages,
// organized by fullness state. Pages are moved between lists as
// allocations and frees change the page's used count.
struct mi_heap_t {
    uint32_t    heap_id;          // unique heap identifier
    uint32_t    cookie;           // encoding cookie (randomized)

    // ─── Page lists ───
    mi_page_t*  pages;            // all pages in this heap (doubly-linked)
    mi_page_t*  pages_free;       // completely free pages (used == 0)
    mi_page_t*  pages_full;       // full pages (used == capacity)

    // ─── Per-size-class quick lookup ───
    mi_page_t*  page_direct[MI_CLASS_COUNT];  // direct pointer to non-full page

    // ─── Delayed free ───
    mi_block_t* delayed_free;     // singly-linked list of blocks to free lazily
    size_t      delayed_count;

    // ─── Heap list ───
    mi_heap_t*  next;             // global heap list
    mi_heap_t*  prev;

    // ─── Statistics ───
    size_t      allocated;        // bytes currently allocated
    size_t      peak;             // peak bytes allocated

    // Create a new heap
    static mi_heap_t* create() noexcept;
    // Destroy a heap (abandon pages)
    void destroy() noexcept;
    // Get thread-local heap (creates if needed)
    static mi_heap_t* get() noexcept;
};

// ─── Global abandoned page list ───
struct mi_abandoned_t {
    std::atomic<mi_page_t*> head{nullptr};
    void push(mi_page_t* page) noexcept;
    mi_page_t* pop() noexcept;
};

extern mi_abandoned_t mi_abandoned;

// ─── Size class helpers ───
inline size_t mi_class_index(size_t size) noexcept {
    if (size == 0) size = 1;
    if (size <= MI_MAX_SMALL_SIZE) {
        size_t idx = (size + MI_CLASS_STEP_SMALL - 1) / MI_CLASS_STEP_SMALL - 1;
        return idx < MI_CLASS_COUNT_SMALL ? idx : MI_CLASS_COUNT_SMALL - 1;
    }
    // Medium: 4KB+ in 128B steps
    if (size <= MI_PAGE_MEDIUM_SIZE) {
        size_t base = (size - MI_MAX_SMALL_SIZE + 127) / 128;
        return MI_CLASS_COUNT_SMALL + (base < MI_CLASS_COUNT_MEDIUM ? base : MI_CLASS_COUNT_MEDIUM - 1);
    }
    return MI_CLASS_COUNT;  // too large, use direct OS allocation
}

inline size_t mi_class_size(size_t idx) noexcept {
    if (idx < MI_CLASS_COUNT_SMALL) {
        return (idx + 1) * MI_CLASS_STEP_SMALL;
    }
    return MI_MAX_SMALL_SIZE + (idx - MI_CLASS_COUNT_SMALL + 1) * 128;
}

// Thread-local heap pointer
extern thread_local mi_heap_t* mi_thread_heap;

// Initialize the thread-local heap
void mi_thread_init() noexcept;

} // namespace mimalloc
} // namespace my_ptmalloc
