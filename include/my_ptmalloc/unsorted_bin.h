#pragma once
// UnsortedBin: single doubly-linked list acting as staging area
// ALL freed chunks (not in fastbin/tcache) go here first
// On malloc, chunks are scanned and sorted into proper small/large bins

#include "config.h"
#include "types.h"
#include "chunk.h"
#include "intrusive_list.h"

namespace my_ptmalloc {

// Forward declarations
class SmallBins;
class LargeBins;

class UnsortedBin {
    IntrusiveList bin_;

public:
    void init() noexcept { bin_.init(); }

    void push(Chunk* p) noexcept {
        // Clear large bin pointers defensively. Chunks entering the unsorted
        // bin may have stale fd_nextsize/bk_nextsize from a previous large bin
        // residence. These fields overlap user data and may have been overwritten.
        // Only touch them for large chunks; small chunks may not have room.
        if (!in_smallbin_range(p->chunk_size())) {
            p->fd_nextsize = nullptr;
            p->bk_nextsize = nullptr;
        }
        bin_.push_front(p);
    }

    [[nodiscard]] Chunk* pop_back() noexcept {
        if (bin_.empty()) return nullptr;
        Chunk* victim = bin_.back();
        bin_.unlink(victim);
        return victim;
    }

    [[nodiscard]] bool empty() const noexcept {
        return bin_.empty();
    }

    [[nodiscard]] bool contains(Chunk* p) const noexcept {
        return bin_.contains(p);
    }

    void unlink(Chunk* p) noexcept {
        bin_.unlink(p);
    }

    // Scan unsorted bin: sort chunks into proper bins, return exact match if found
    // Also fills tcache during scan
    // Returns matching chunk or nullptr
    [[nodiscard]] Chunk* scan_and_sort(
        ChunkSize nb, SmallBins& sb, LargeBins& lb,
        void* tcache_ptr, Chunk*& last_remainder) noexcept;

    [[nodiscard]] IntrusiveList& list() noexcept { return bin_; }
};

} // namespace my_ptmalloc
