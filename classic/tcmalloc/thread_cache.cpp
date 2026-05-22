// ThreadCache implementation.

#include "thread_cache.h"
#include "page_heap.h"

#include <cstdlib>
#include <cstring>
#include <new>

namespace my_ptmalloc {
namespace tcmalloc {

thread_local ThreadCache* tc_thread_cache = nullptr;

ThreadCache::ThreadCache() noexcept
    : allocated(0), max_allocated(4 * 1024 * 1024), next(nullptr), prev(nullptr)
{
    for (unsigned i = 0; i < kNumClasses; i++) {
        list[i] = nullptr;
        count[i] = 0;
        max_count[i] = 2;   // slow-start: begin small
        low_water[i] = 0;
    }
}

void tc_init() noexcept {
    if (tc_thread_cache) return;
    void* mem = std::aligned_alloc(64, sizeof(ThreadCache));
    if (!mem) return;
    tc_thread_cache = new (mem) ThreadCache();
}

void* ThreadCache::alloc(size_t class_idx) noexcept {
    if (class_idx >= kNumClasses) return nullptr;

    // Pop from local freelist
    if (list[class_idx]) {
        FreeNode* node = list[class_idx];
        list[class_idx] = node->next;
        count[class_idx]--;
        allocated -= kClassInfo[class_idx].size;
        return static_cast<void*>(node);
    }

    // Refill from central freelist
    refill(class_idx);
    if (list[class_idx]) {
        FreeNode* node = list[class_idx];
        list[class_idx] = node->next;
        count[class_idx]--;
        allocated -= kClassInfo[class_idx].size;
        return static_cast<void*>(node);
    }

    return nullptr;  // OOM
}

bool ThreadCache::free(size_t class_idx, void* ptr) noexcept {
    if (class_idx >= kNumClasses) return false;

    auto* node = static_cast<FreeNode*>(ptr);

    // If we have too many, scavenge some back to central
    if (count[class_idx] >= max_count[class_idx]) {
        scavenge();
    }

    // Push to local freelist
    node->next = list[class_idx];
    list[class_idx] = node;
    count[class_idx]++;
    allocated += kClassInfo[class_idx].size;

    // Trigger GC if needed
    if (allocated > max_allocated) {
        scavenge();
    }

    return true;
}

void ThreadCache::refill(size_t class_idx) noexcept {
    // Batch transfer from central freelist
    size_t batch = max_count[class_idx];
    if (batch < kClassInfo[class_idx].num_to_move) {
        batch = kClassInfo[class_idx].num_to_move;
    }

    size_t got = 0;
    FreeNode* head = g_central[class_idx].remove_range(batch, &got);
    if (!head) return;

    // Prepend to local list
    FreeNode* tail = head;
    while (tail->next) tail = tail->next;
    tail->next = list[class_idx];
    list[class_idx] = head;
    count[class_idx] += got;

    // Update slow-start
    update_capacity(class_idx);
}

void ThreadCache::scavenge() noexcept {
    // Return excess objects to central freelist
    for (unsigned i = 0; i < kNumClasses; i++) {
        if (count[i] > low_water[i]) {
            size_t to_return = count[i] - low_water[i];
            if (to_return > 0) {
                FreeNode* head = list[i];
                FreeNode* tail = head;
                for (size_t j = 1; j < to_return && tail->next; j++) {
                    tail = tail->next;
                }
                list[i] = tail->next;
                tail->next = nullptr;
                count[i] -= to_return;
                g_central[i].insert_range(head, tail, to_return);
            }
        }
    }
    allocated = 0;
}

void ThreadCache::update_capacity(size_t class_idx) noexcept {
    // Slow-start: if we used close to capacity, increase it
    if (count[class_idx] >= max_count[class_idx] * 3 / 4) {
        max_count[class_idx] = max_count[class_idx] * 2;
        if (max_count[class_idx] > 8192) max_count[class_idx] = 8192;
    }
    low_water[class_idx] = max_count[class_idx] / 4;
}

} // namespace tcmalloc
} // namespace my_ptmalloc
