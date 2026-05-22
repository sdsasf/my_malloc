// PageHeap implementation.

#include "page_heap.h"
#include "central_freelist.h"

#include <sys/mman.h>
#include <cstring>
#include <algorithm>

namespace my_ptmalloc {
namespace tcmalloc {

PageHeap g_page_heap;

PageHeap::PageHeap() noexcept
    : large_set(nullptr), system_bytes(0), free_bytes(0)
{
    pthread_mutex_init(&lock, nullptr);
    std::memset(free_, 0, sizeof(free_));
}

Span* PageHeap::alloc_span(size_t n) noexcept {
    pthread_mutex_lock(&lock);

    // Search free lists for exact or larger span
    Span* best = nullptr;
    for (size_t i = n; i <= kMaxPages; i++) {
        if (free_[i]) {
            best = free_[i];
            // Unlink
            free_[i] = best->next;
            if (best->next) best->next->prev = nullptr;
            break;
        }
    }

    if (!best) {
        best = grow_heap(n > kMaxPages ? n : kMaxPages);
        if (!best) {
            pthread_mutex_unlock(&lock);
            return nullptr;
        }
    }

    // If span is larger than needed, split
    if (best->length > n) {
        Span* remainder = new Span();
        remainder->start   = best->start + n;
        remainder->length  = best->length - n;
        remainder->state   = SpanState::ON_NORMAL_FREELIST;
        remainder->size_class = kNumClasses;

        // Insert remainder into appropriate free list
        size_t rlen = remainder->length;
        if (rlen <= kMaxPages) {
            remainder->next = free_[rlen];
            if (free_[rlen]) free_[rlen]->prev = remainder;
            free_[rlen] = remainder;
        } else {
            remainder->next = large_set;
            large_set = remainder;
        }

        best->length = n;
    }

    best->state = SpanState::IN_USE;
    best->size_class = kNumClasses;

    // Update PageMap
    pagemap.map_span(best);

    free_bytes -= best->length * kPageSize;
    pthread_mutex_unlock(&lock);
    return best;
}

void PageHeap::free_span(Span* span) noexcept {
    pthread_mutex_lock(&lock);
    span->state = SpanState::ON_NORMAL_FREELIST;
    span->size_class = kNumClasses;

    free_bytes += span->length * kPageSize;

    // Try to coalesce
    Span* merged = coalesce(span);

    // Insert into free list
    size_t len = merged->length;
    if (len <= kMaxPages) {
        merged->next = free_[len];
        if (free_[len]) free_[len]->prev = merged;
        merged->prev = nullptr;
        free_[len] = merged;
    } else {
        merged->next = large_set;
        merged->prev = nullptr;
        large_set = merged;
    }

    pthread_mutex_unlock(&lock);
}

Span* PageHeap::alloc_large(size_t bytes) noexcept {
    size_t pages = (bytes + kPageSize - 1) >> Span::kPageShift;
    Span* span = alloc_span(pages);
    if (span) span->size_class = kNumClasses;  // large
    return span;
}

void PageHeap::free_large(Span* span) noexcept {
    free_span(span);
}

Span* PageHeap::grow_heap(size_t n) noexcept {
    size_t bytes = n * kPageSize;
    void* raw = ::mmap(nullptr, bytes, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) return nullptr;

    Span* span = new Span();
    span->start  = Span::page_number(raw);
    span->length = n;
    span->state  = SpanState::IN_USE;
    span->refcount = 0;
    span->freelist  = nullptr;
    span->prev = span->next = nullptr;

    system_bytes += bytes;
    pagemap.map_span(span);
    return span;
}

Span* PageHeap::coalesce(Span* span) noexcept {
    // Check adjacent pages for free spans
    if (span->start > 0) {
        Span* prev = pagemap.get(span->start - 1);
        if (prev && prev->state == SpanState::ON_NORMAL_FREELIST
            && prev->start + prev->length == span->start) {
            // Merge with previous
            // Unlink prev from its free list
            if (prev->prev) prev->prev->next = prev->next;
            else {
                size_t plen = prev->length;
                if (plen <= kMaxPages) free_[plen] = prev->next;
            }
            if (prev->next) prev->next->prev = prev->prev;

            span->start = prev->start;
            span->length += prev->length;
            delete prev;
        }
    }

    Span* next = pagemap.get(span->start + span->length);
    if (next && next->state == SpanState::ON_NORMAL_FREELIST) {
        // Merge with next
        if (next->prev) next->prev->next = next->next;
        else {
            size_t nlen = next->length;
            if (nlen <= kMaxPages) free_[nlen] = next->next;
        }
        if (next->next) next->next->prev = next->prev;

        span->length += next->length;
        delete next;
    }

    return span;
}

void PageHeap::release_to_os() noexcept {
    pthread_mutex_lock(&lock);
    // Release spans over a certain threshold back to OS
    for (size_t i = kMaxPages / 2; i <= kMaxPages; i++) {
        Span* s = free_[i];
        while (s) {
            Span* next = s->next;
            if (s->length >= kMaxPages / 2) {
                // Unlink and munmap
                if (s->prev) s->prev->next = s->next;
                else free_[i] = s->next;
                if (s->next) s->next->prev = s->prev;

                void* addr = Span::page_addr(s->start);
                size_t sz = s->length * kPageSize;
                ::munmap(addr, sz);
                system_bytes -= sz;
                free_bytes -= sz;
                delete s;
            }
            s = next;
        }
    }
    pthread_mutex_unlock(&lock);
}

void Span::build_freelist(size_t object_size) noexcept {
    void* base = Span::page_addr(start);
    size_t num_objects = (length * kPageSize) / object_size;

    freelist = nullptr;
    for (size_t i = 0; i < num_objects; i++) {
        auto* node = reinterpret_cast<FreeNode*>(
            static_cast<char*>(base) + i * object_size);
        node->next = freelist;
        freelist = node;
    }
    refcount = 0;
}

} // namespace tcmalloc
} // namespace my_ptmalloc
