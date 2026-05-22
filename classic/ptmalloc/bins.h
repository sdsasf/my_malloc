#pragma once
// ptmalloc bins: fastbins, smallbins, largebins, unsorted bin.
// Faithful to glibc's bin organization.

#include "config.h"
#include "chunk.h"

#include <atomic>
#include <cstddef>

namespace my_ptmalloc {
namespace ptmalloc {

struct Tcache;

// ─── Bin index mapping ───
// bin 0:   unused
// bin 1:   unsorted bin (circular doubly-linked)
// bin 2-65:  smallbins (NSMALLBINS=64)
// bin 65-127: largebins (NLARGEBINS=63, indexed as largebin_index)

constexpr unsigned UNSORTED_BIN_IDX = 1;

// ─── Fastbin (singly-linked LIFO, CAS-based lock-free) ───
struct Fastbin {
    std::atomic<Chunk*> head{nullptr};

    // CAS push (lock-free)
    void push(Chunk* p) noexcept;
    // CAS pop (lock-free)
    Chunk* pop(FastbinIdx fidx) noexcept;
    // Non-atomic pop (used during consolidation under lock)
    Chunk* pop_all() noexcept;
    bool empty() const noexcept { return head.load(std::memory_order_acquire) == nullptr; }
};

// ─── Bin list node (circular doubly-linked list) ───
// Each bin is represented by a pair of fd/bk pointers. An empty bin
// points to itself (bin->fd == bin->bk == bin_as_chunk).
// "bin_as_chunk" means we cast the fd/bk storage to a Chunk* and
// treat it as the list head sentinel.

struct BinList {
    Chunk* fd;  // forward (first chunk in list)
    Chunk* bk;  // backward (last chunk in list)

    void init() noexcept {
        fd = bk = as_chunk();
    }

    bool empty() const noexcept {
        return fd == as_chunk();
    }

    // Push to front (unsorted bin uses this for inserting)
    void push_front(Chunk* p) noexcept;
    // Push to back (smallbins use this for FIFO behavior)
    void push_back(Chunk* p) noexcept;
    // Unlink a chunk from this list
    void unlink(Chunk* p) noexcept;
    // Pop from front
    Chunk* pop_front() noexcept;

    // Get the head of the list
    Chunk* head() const noexcept { return fd != as_chunk() ? fd : nullptr; }

    // Cast bin storage to Chunk* for sentinel
    Chunk* as_chunk() noexcept {
        // We embed fd/bk at the same offsets they'd have in a Chunk.
        // So we can cast &fd to Chunk* (fd is at offset 16 in Chunk = prev_size + size).
        // This is the glibc trick: bin_at(m, i) macro.
        return reinterpret_cast<Chunk*>(reinterpret_cast<char*>(&fd) - 2 * SIZE_SZ);
    }
    const Chunk* as_chunk() const noexcept {
        return reinterpret_cast<const Chunk*>(reinterpret_cast<const char*>(&fd) - 2 * SIZE_SZ);
    }
};

// ─── Largebin entry ───
// Each largebin holds a range of chunk sizes, sorted descending.
// fd_nextsize/bk_nextsize form a secondary list grouping by distinct size,
// skipping chunks of the same size.
struct LargebinRange {
    unsigned min_size;
    unsigned max_size;
};

// ─── BinManager ───
class BinManager {
public:
    BinManager() noexcept;

    // ─── Fastbins ───
    Fastbin& fast(FastbinIdx idx) noexcept {
        return fastbins_[idx.value];
    }
    const Fastbin& fast(FastbinIdx idx) const noexcept {
        return fastbins_[idx.value];
    }
    bool has_fastchunks() const noexcept;

    // ─── Unsorted bin ───
    BinList& unsorted() noexcept { return bins_[UNSORTED_BIN_IDX]; }
    const BinList& unsorted() const noexcept { return bins_[UNSORTED_BIN_IDX]; }

    // ─── Smallbins ───
    BinList& small(SmallbinIdx idx) noexcept {
        return bins_[UNSORTED_BIN_IDX + 1 + idx.value];
    }
    // Allocate from smallbin (exact-fit FIFO)
    Chunk* alloc(SmallbinIdx idx) noexcept;

    // ─── Largebins ───
    BinList& large(unsigned li) noexcept {
        return bins_[UNSORTED_BIN_IDX + 1 + NSMALLBINS + li];
    }
    const BinList& large(unsigned li) const noexcept {
        return bins_[UNSORTED_BIN_IDX + 1 + NSMALLBINS + li];
    }

    // Allocate from largebins (best-fit, may split)
    struct LargeAllocResult {
        Chunk* victim;
        Chunk* remainder;
    };
    LargeAllocResult alloc_split(ChunkSize nb, BinList& unsorted_bin) noexcept;

    // ─── Bin map (bitmap of non-empty bins) ───
    void mark_bin(unsigned bin_idx) noexcept;
    void unmark_bin(unsigned bin_idx) noexcept;
    [[nodiscard]] unsigned next_nonempty(unsigned start) const noexcept;
    [[nodiscard]] bool bin_nonempty(unsigned bin_idx) const noexcept;

    // ─── Utility ───
    // Check if a chunk is in any free bin
    bool contains_free_chunk(Chunk* p) const noexcept;

    // Unlink a free chunk from whichever bin it's in
    bool unlink_free_chunk(Chunk* p) noexcept;

    // Initialize all bins
    void init() noexcept;

    // Total free chunks count (for stats)
    size_t free_chunk_count() const noexcept;

private:
    Fastbin     fastbins_[NFASTBINS];
    BinList     bins_[N_BINS];          // bin 0 unused, bin 1 unsorted, bins 2..127
    unsigned    binmap_[BINMAPSIZE];     // bitmap tracking non-empty bins

    // Largebin ranges (glibc's largebin_index computation)
    static const LargebinRange largebin_ranges_[NLARGEBINS];

public:
    [[nodiscard]] unsigned largebin_index(ChunkSize sz) const noexcept;
    [[nodiscard]] static const LargebinRange& largebin_range(unsigned li) noexcept;
};

} // namespace ptmalloc
} // namespace my_ptmalloc
