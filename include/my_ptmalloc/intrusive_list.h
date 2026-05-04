#pragma once
// Intrusive doubly-linked circular list for bin management
// Sentinel node acts as circular anchor; empty list = sentinel points to itself

#include "chunk.h"
#include <cassert>
#include <cstdio>

namespace my_ptmalloc {

class IntrusiveList {
    Chunk sentinel_;

public:
    void init() noexcept {
        // Sentinel: prev_size=0, size=0, fd=bk=self
        sentinel_.prev_size = 0;
        sentinel_.size = 0;
        sentinel_.fd = &sentinel_;
        sentinel_.bk = &sentinel_;
        sentinel_.fd_nextsize = nullptr;
        sentinel_.bk_nextsize = nullptr;
    }

    IntrusiveList() noexcept { init(); }

    [[nodiscard]] bool empty() const noexcept {
        return sentinel_.fd == &sentinel_;
    }

    // FIFO: take from back (most recently added for unsorted, FIFO for small)
    [[nodiscard]] Chunk* back() const noexcept {
        return sentinel_.bk;
    }

    // LIFO: take from front
    [[nodiscard]] Chunk* front() const noexcept {
        return sentinel_.fd;
    }

    // Push to front (LIFO order)
    void push_front(Chunk* p) noexcept {
        if (!p->is_valid()) {
#if MY_PTMALLOC_DEBUG
            fprintf(stderr, "[IntrusiveList::push_front] SKIP invalid chunk %p raw=0x%zx prev=0x%zx\n",
                    (void*)p, p->size, p->prev_size);
#endif
            return;
        }
        p->fd = sentinel_.fd;
        p->bk = &sentinel_;
        sentinel_.fd->bk = p;
        sentinel_.fd = p;
    }

    // Push to back (FIFO order)
    void push_back(Chunk* p) noexcept {
        p->fd = &sentinel_;
        p->bk = sentinel_.bk;
        sentinel_.bk->fd = p;
        sentinel_.bk = p;
    }

    // Unlink a chunk from this list with integrity check
    void unlink(Chunk* p) noexcept {
        Chunk* fd = p->fd;
        Chunk* bk = p->bk;
        // Integrity check: doubly-linked list consistency
        assert(fd->bk == p && bk->fd == p);
        fd->bk = bk;
        bk->fd = fd;
    }

    [[nodiscard]] bool contains(Chunk* p) const noexcept {
        const Chunk* sentinel = &sentinel_;
        for (const Chunk* cur = sentinel_.fd; cur != sentinel; cur = cur->fd) {
            if (cur == p) return true;
        }
        return false;
    }

    // Get sentinel for raw access (used by bin_at macros for compatibility)
    [[nodiscard]] Chunk* sentinel() noexcept { return &sentinel_; }
    [[nodiscard]] const Chunk* sentinel() const noexcept { return &sentinel_; }
};

} // namespace my_ptmalloc
