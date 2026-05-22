// CentralFreeList implementation.

#include "central_freelist.h"

namespace my_ptmalloc {
namespace tcmalloc {

CentralFreeList g_central[kNumClasses];

CentralFreeList::CentralFreeList() noexcept
    : nonempty(nullptr), empty(nullptr), tc_length(1)
{
    pthread_mutex_init(&lock, nullptr);
}

FreeNode* CentralFreeList::remove_range(size_t N, size_t* count) noexcept {
    pthread_mutex_lock(&lock);

    // Find a non-empty span
    Span* span = nonempty;
    if (!span) {
        pthread_mutex_unlock(&lock);
        *count = 0;
        return nullptr;
    }

    FreeNode* head = nullptr;
    FreeNode* tail = nullptr;
    size_t removed = 0;

    while (span && removed < N) {
        while (span->freelist && removed < N) {
            FreeNode* node = span->freelist;
            span->freelist = node->next;
            span->refcount++;

            if (!head) head = tail = node;
            else { tail->next = node; tail = node; }
            node->next = nullptr;
            removed++;
        }

        if (!span->freelist) {
            // Span is now full — move to empty list
            // Unlink from nonempty
            if (span->prev) span->prev->next = span->next;
            else nonempty = span->next;
            if (span->next) span->next->prev = span->prev;

            // Push to empty list
            span->prev = nullptr;
            span->next = empty;
            if (empty) empty->prev = span;
            empty = span;

            span = nonempty;
        } else {
            break;  // done
        }
    }

    pthread_mutex_unlock(&lock);
    *count = removed;
    return head;
}

void CentralFreeList::insert_range(FreeNode* head, FreeNode* tail, size_t N) noexcept {
    pthread_mutex_lock(&lock);

    (void)N;
    FreeNode* cur = head;
    while (cur) {
        FreeNode* next = cur->next;
        // Find the owning span for this node via PageMap lookup
        Span* span = nullptr;  // Will be set by caller context — actually, we know the span
        // For simplicity, insert all back to first non-empty span
        // Real tcmalloc uses PageMap to find the owning span.
        // This simplified version returns chunks to the first non-full span.
        if (nonempty && nonempty->size_class < kNumClasses) {
            span = nonempty;
        } else {
            // Move an empty span back to nonempty
            span = empty;
            if (span) {
                empty = span->next;
                if (empty) empty->prev = nullptr;
                span->next = nonempty;
                if (nonempty) nonempty->prev = span;
                span->prev = nullptr;
                nonempty = span;
            }
        }
        if (span) {
            span->refcount--;
            if (span->refcount == 0 && span->next) {
                // Span is now empty — can be returned to PageHeap
            }
        }
        cur = next;
    }

    pthread_mutex_unlock(&lock);
}

void CentralFreeList::add_span(Span* span) noexcept {
    pthread_mutex_lock(&lock);
    span->prev = nullptr;
    span->next = nonempty;
    if (nonempty) nonempty->prev = span;
    nonempty = span;
    pthread_mutex_unlock(&lock);
}

void CentralFreeList::update_tc_length(size_t actual) noexcept {
    (void)actual;
    // Slow-start: if we got fewer than requested, reduce batch size.
    // If we got exactly what we requested, grow batch size.
    if (actual >= tc_length) {
        tc_length = tc_length * 2;
        if (tc_length > 8192) tc_length = 8192;
    } else if (actual > 0) {
        tc_length = actual;
    }
}

} // namespace tcmalloc
} // namespace my_ptmalloc
