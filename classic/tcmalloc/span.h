#pragma once
// Span: a contiguous run of pages managed by PageHeap.

#include "size_classes.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {
namespace tcmalloc {

struct Span;

// Free list node within a span
struct FreeNode {
    FreeNode* next;
};

// Span state
enum class SpanState : uint8_t {
    IN_USE               = 0,  // allocated to an application
    ON_NORMAL_FREELIST   = 1,  // on PageHeap's free list
    ON_RETURNED_FREELIST = 2,  // returned to OS, retained for reuse
};

// Span: a contiguous run of `length` pages starting at `start`.
struct Span {
    uintptr_t    start;         // first page number (page-aligned address >> kPageShift)
    size_t       length;        // number of pages
    unsigned     size_class;    // 0=kNumClasses=large, otherwise class index
    SpanState    state;
    uint32_t     refcount;      // number of allocated objects in this span

    // Free list of available objects within this span
    FreeNode*    freelist;

    // Span list links
    Span*        prev;
    Span*        next;

    // ─── Helpers ───
    static constexpr size_t kPageShift = 13;  // 8KB pages
    static constexpr size_t kPageMask  = kPageSize - 1;

    // Page number from address
    static uintptr_t page_number(void* p) noexcept {
        return reinterpret_cast<uintptr_t>(p) >> kPageShift;
    }
    // Address from page number
    static void* page_addr(uintptr_t pn) noexcept {
        return reinterpret_cast<void*>(pn << kPageShift);
    }

    // Does this span own a given page?
    bool contains_page(uintptr_t pn) const noexcept {
        return pn >= start && pn < start + length;
    }

    // Build free list from span's memory
    void build_freelist(size_t object_size) noexcept;
};

} // namespace tcmalloc
} // namespace my_ptmalloc
