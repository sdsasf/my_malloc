// Segment implementation.

#include "segment.h"
#include "os.h"

#include <cstring>

namespace my_ptmalloc {
namespace mimalloc {

thread_local mi_segment_cache_t mi_seg_cache{nullptr, 0};

void mi_segment_cache_t::push(mi_segment_t* seg) noexcept {
    if (count >= MAX_CACHED) {
        OS::segment_free(seg);
        return;
    }
    seg->next = head;
    head = seg;
    count++;
}

mi_segment_t* mi_segment_cache_t::pop() noexcept {
    if (!head) return nullptr;
    mi_segment_t* seg = head;
    head = seg->next;
    seg->next = nullptr;
    count--;
    return seg;
}

mi_segment_t* mi_segment_alloc(size_t page_count) noexcept {
    // Try cache first
    mi_segment_t* seg = mi_seg_cache.pop();
    if (!seg) {
        seg = static_cast<mi_segment_t*>(OS::segment_alloc());
        if (!seg) return nullptr;
    }

    seg->magic = 0x6d696d616c6c6f63ULL;  // "mimalloc"
    seg->next = nullptr;
    seg->abandoned = 0;
    seg->commit_mask = 0;

    // Calculate page capacity
    size_t metadata_size = sizeof(mi_segment_t) + sizeof(mi_page_t) * page_count;
    size_t usable = MI_SEGMENT_SIZE - metadata_size;
    seg->page_capacity = static_cast<uint32_t>(page_count);
    seg->page_count = 0;

    std::memset(seg->pages, 0, sizeof(mi_page_t) * page_count);
    return seg;
}

void mi_segment_free(mi_segment_t* seg) noexcept {
    seg->magic = 0;
    mi_seg_cache.push(seg);
}

mi_page_t* mi_segment_page_of(void* p) noexcept {
    return mi_segment_t::page_of(p);
}

} // namespace mimalloc
} // namespace my_ptmalloc
