#pragma once
// Segment management: allocation, page lookup, segment cache.

#include "types.h"

namespace my_ptmalloc {
namespace mimalloc {

// ─── Segment cache (per-thread) ───
// Keeps recently freed segments for fast reuse.
struct mi_segment_cache_t {
    mi_segment_t* head;
    size_t        count;
    static constexpr size_t MAX_CACHED = 8;

    void push(mi_segment_t* seg) noexcept;
    mi_segment_t* pop() noexcept;
};

extern thread_local mi_segment_cache_t mi_seg_cache;

// ─── Segment operations ───
mi_segment_t* mi_segment_alloc(size_t page_count) noexcept;
void mi_segment_free(mi_segment_t* seg) noexcept;

// Given a pointer, find the owning page via segment lookup
mi_page_t* mi_segment_page_of(void* p) noexcept;

} // namespace mimalloc
} // namespace my_ptmalloc
