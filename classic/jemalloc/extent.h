#pragma once
// extent_t: a contiguous range of pages, managed by buddy allocator.
// States: dirty → muzzy → clean → retained (progressive decay).

#include "size_classes.h"

#include <cstdint>
#include <cstddef>
#include <atomic>

namespace my_ptmalloc {
namespace jemalloc {

enum class ExtentState : uint8_t {
    DIRTY    = 0,  // recently freed, may contain data
    MUZZY    = 1,  // zero-filled, not yet returned to OS
    CLEAN    = 2,  // never used or fully purged
    RETAINED = 3,  // returned to OS, virtual address retained
};

struct extent_t {
    void*        addr;       // base address (page-aligned)
    size_t       size;       // size in bytes (multiple of page_size)
    size_t       usize;      // usable size (for large objects)
    ExtentState  state;
    unsigned     arena_idx;  // owning arena
    uint32_t     nfree;      // number of free objects (for runs)

    // Tree links (stored in RB-tree by arena)
    extent_t*    prev;
    extent_t*    next;

    // Buddy allocator links
    extent_t*    buddy_prev;
    extent_t*    buddy_next;

    // Run metadata (if this extent is used as a run of small objects)
    void*        run_bitmap;  // bitmap tracking free/in-use slots
    size_t       run_nslots;  // slots per run
    size_t       run_obj_size; // size of each object in this run
};

// ─── Extent helpers ───
extent_t* extent_alloc(size_t pages) noexcept;
void extent_free(extent_t* ext) noexcept;

// Buddy allocator: find extent in tree, split/merge
void extent_tree_insert(extent_t** root, extent_t* ext) noexcept;
void extent_tree_remove(extent_t** root, extent_t* ext) noexcept;
extent_t* extent_tree_find(extent_t* root, size_t pages) noexcept;

} // namespace jemalloc
} // namespace my_ptmalloc
