#pragma once
// FastBins: small chunks (<=160 bytes on 64-bit) in singly-linked LIFO lists
// Lock-free CAS operation for multi-threaded performance
// NO coalescing: fastbin chunks are never merged with neighbors

#include "config.h"
#include "types.h"
#include "chunk.h"
#include <atomic>
#include <functional>

namespace my_ptmalloc {

class alignas(64) FastBins {
    std::atomic<Chunk*> heads_[NFASTBINS]{};

public:
    void init() noexcept {
        for (auto& h : heads_) h.store(nullptr, std::memory_order_relaxed);
    }

    // Pop from front (LIFO) -- CAS lock-free
    [[nodiscard]] Chunk* pop(FastbinIdx idx) noexcept {
        Chunk* head = heads_[idx.value].load(std::memory_order_acquire);
        while (head != nullptr) {
            Chunk* next = head->fd;
            if (heads_[idx.value].compare_exchange_weak(
                    head, next,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire)) {
                return head;
            }
            // head was updated by CAS failure, retry
        }
        return nullptr;
    }

    // Push to front (LIFO) -- CAS lock-free
    void push(FastbinIdx idx, Chunk* p) noexcept {
        Chunk* old_head = heads_[idx.value].load(std::memory_order_relaxed);
        do {
            p->fd = old_head;
        } while (!heads_[idx.value].compare_exchange_weak(
                    old_head, p,
                    std::memory_order_acq_rel,
                    std::memory_order_relaxed));
    }

    [[nodiscard]] bool has_chunks(FastbinIdx idx) const noexcept {
        return heads_[idx.value].load(std::memory_order_acquire) != nullptr;
    }

    // Clear all fastbins (for consolidation)
    void clear_all() noexcept {
        for (auto& h : heads_) h.store(nullptr, std::memory_order_relaxed);
    }

    // Drain all bins, calling callback for each chunk
    void drain(std::function<void(Chunk*)> callback) noexcept {
        for (size_t i = 0; i < NFASTBINS; ++i) {
            Chunk* p = heads_[i].exchange(nullptr, std::memory_order_acq_rel);
            while (p != nullptr) {
                Chunk* next = p->fd;
                callback(p);
                p = next;
            }
        }
    }

    // Get raw head pointer (for consolidation scanning)
    [[nodiscard]] Chunk* peek(FastbinIdx idx) const noexcept {
        return heads_[idx.value].load(std::memory_order_acquire);
    }
};

} // namespace my_ptmalloc
