// ptmalloc bins implementation: fastbins, smallbins, largebins, unsorted bin.

#include "bins.h"
#include "chunk.h"

#include <cstring>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Fastbin ───

void Fastbin::push(Chunk* p) noexcept {
    Chunk* old = head.load(std::memory_order_acquire);
    do {
        // fd of chunk points to old head
        p->fd = old;
    } while (!head.compare_exchange_weak(old, p,
             std::memory_order_release, std::memory_order_acquire));
}

Chunk* Fastbin::pop(FastbinIdx fidx) noexcept {
    (void)fidx;
    Chunk* p = head.load(std::memory_order_acquire);
    while (p) {
        Chunk* next = p->fd;
        if (head.compare_exchange_weak(p, next,
                std::memory_order_release, std::memory_order_acquire)) {
            return p;
        }
    }
    return nullptr;
}

Chunk* Fastbin::pop_all() noexcept {
    return head.exchange(nullptr, std::memory_order_acquire);
}

// ─── BinList ───

void BinList::push_front(Chunk* p) noexcept {
    Chunk* sentinel = as_chunk();
    Chunk* first = fd;

    p->fd = first;
    p->bk = sentinel;
    sentinel->fd = p;
    if (first) first->bk = p;
}

void BinList::push_back(Chunk* p) noexcept {
    Chunk* sentinel = as_chunk();
    Chunk* last = bk;

    p->fd = sentinel;
    p->bk = last;
    sentinel->bk = p;
    if (last) last->fd = p;
}

void BinList::unlink(Chunk* p) noexcept {
    // The chunk must be in a doubly-linked list.
    // glibc unlink macro with security checks.
    Chunk* fwd = p->fd;
    Chunk* bck = p->bk;

    // Safety check: fd->bk == p && bk->fd == p
    if (fwd && fwd->bk == p && bck && bck->fd == p) {
        fwd->bk = bck;
        bck->fd = fwd;
    }
    // In production glibc this is a fatal error; we're lenient here.
}

Chunk* BinList::pop_front() noexcept {
    if (empty()) return nullptr;
    Chunk* p = fd;
    unlink(p);
    return p;
}

// ─── Largebin ranges (glibc compatible) ───
const LargebinRange BinManager::largebin_ranges_[NLARGEBINS] = {
    {  1024,   1088 },  // bin 0:   1024..1087
    {  1088,   1152 },  // bin 1
    {  1152,   1216 },
    {  1216,   1280 },
    {  1280,   1344 },
    {  1344,   1408 },
    {  1408,   1472 },
    {  1472,   1536 },
    {  1536,   1600 },
    {  1600,   1664 },
    {  1664,   1728 },
    {  1728,   1792 },
    {  1792,   1856 },
    {  1856,   1920 },
    {  1920,   1984 },
    {  1984,   2048 },
    {  2048,   2176 },  // bin 16: wider ranges
    {  2176,   2304 },
    {  2304,   2432 },
    {  2432,   2560 },
    {  2560,   2688 },
    {  2688,   2816 },
    {  2816,   2944 },
    {  2944,   3072 },
    {  3072,   3328 },  // bin 24
    {  3328,   3584 },
    {  3584,   3840 },
    {  3840,   4096 },
    {  4096,   4608 },
    {  4608,   5120 },
    {  5120,   5632 },
    {  5632,   6144 },
    {  6144,   6656 },  // bin 32
    {  6656,   7168 },
    {  7168,   7680 },
    {  7680,   8192 },
    {  8192,   9216 },
    {  9216,  10240 },
    { 10240,  11264 },
    { 11264,  12288 },
    { 12288,  14336 },  // bin 40
    { 14336,  16384 },
    { 16384,  18432 },
    { 18432,  20480 },
    { 20480,  24576 },
    { 24576,  28672 },
    { 28672,  32768 },
    { 32768,  40960 },
    { 40960,  49152 },  // bin 48
    { 49152,  57344 },
    { 57344,  65536 },
    { 65536,  81920 },
    { 81920,  98304 },
    { 98304, 114688 },
    {114688, 131072 },
    {131072, 163840 },
    {163840, 196608 },
    {196608, 229376 },
    {229376, 262144 },
    {262144, 327680 },
    {327680, 393216 },
    {393216, 458752 },
    {458752, 524288 },  // bin 62
};

unsigned BinManager::largebin_index(ChunkSize sz) const noexcept {
    size_t s = sz.value;
    // Linear scan over largebin ranges to find the matching bin.
    // For learning purposes this is clear; real glibc uses a computed formula.
    for (unsigned i = 0; i < NLARGEBINS; ++i) {
        if (s < largebin_ranges_[i].max_size) return i;
    }
    return NLARGEBINS - 1;  // largest bin
}

const LargebinRange& BinManager::largebin_range(unsigned li) noexcept {
    return largebin_ranges_[li];
}

// ─── BinManager ───

BinManager::BinManager() noexcept {
    init();
}

void BinManager::init() noexcept {
    // Clear fastbins
    for (unsigned i = 0; i < NFASTBINS; ++i) {
        fastbins_[i].head.store(nullptr, std::memory_order_release);
    }
    // Initialize all bins (circular doubly-linked sentinel)
    for (unsigned i = 0; i < N_BINS; ++i) {
        bins_[i].init();
    }
    // Clear bin map
    for (unsigned i = 0; i < BINMAPSIZE; ++i) {
        binmap_[i] = 0;
    }
}

bool BinManager::has_fastchunks() const noexcept {
    for (unsigned i = 0; i < NFASTBINS; ++i) {
        if (!fastbins_[i].empty()) return true;
    }
    return false;
}

void BinManager::mark_bin(unsigned bin_idx) noexcept {
    unsigned idx = bin_idx >> 5;  // / 32
    unsigned bit = bin_idx & 31;
    binmap_[idx] |= (1u << bit);
}

void BinManager::unmark_bin(unsigned bin_idx) noexcept {
    unsigned idx = bin_idx >> 5;
    unsigned bit = bin_idx & 31;

    // Only clear if the bin is actually empty
    unsigned actual_idx = bin_idx;
    if (actual_idx < N_BINS && bins_[actual_idx].empty()) {
        binmap_[idx] &= ~(1u << bit);
    }
}

unsigned BinManager::next_nonempty(unsigned start) const noexcept {
    // Scan binmap for the next set bit
    for (unsigned word = start >> 5; word < BINMAPSIZE; ++word) {
        if (binmap_[word] == 0) continue;
        unsigned bit_in_word = (word == start >> 5) ? (start & 31) : 0;
        for (unsigned b = bit_in_word; b < 32; ++b) {
            if (binmap_[word] & (1u << b)) {
                unsigned bin_idx = (word << 5) | b;
                if (bin_idx >= start && bin_idx < N_BINS) return bin_idx;
            }
        }
    }
    return N_BINS;
}

bool BinManager::bin_nonempty(unsigned bin_idx) const noexcept {
    if (bin_idx >= N_BINS) return false;
    return !bins_[bin_idx].empty();
}

Chunk* BinManager::alloc(SmallbinIdx idx) noexcept {
    BinList& bin = small(idx);
    if (bin.empty()) return nullptr;

    // FIFO: take from back (oldest entry)
    Chunk* p = bin.bk;
    if (p == bin.as_chunk()) return nullptr;

    bin.unlink(p);
    unmark_bin(UNSORTED_BIN_IDX + 1 + idx.value);
    return p;
}

BinManager::LargeAllocResult BinManager::alloc_split(ChunkSize nb, BinList& unsorted_bin) noexcept {
    // Best-fit search across largebins
    unsigned li = largebin_index(nb);

    // Scan from the first bin that could contain our size up
    for (unsigned i = li; i < NLARGEBINS; ++i) {
        BinList& bin = large(i);
        if (bin.empty()) continue;

        // Largebins are sorted descending by size.
        // Walk from largest (fd) to smallest (bk) to find first-fit >= nb.
        // Actually, in glibc, the largebin list sorted ascending from bk to fd.
        // So the smallest chunk is bk_nextsize, largest is fd.
        // We want best-fit: first chunk >= nb as we walk from smallest to largest.
        // So we start from bk (smallest) and walk towards fd (largest).

        Chunk* sentinel = bin.as_chunk();
        Chunk* p = bin.bk;  // smallest in bin

        // Walk forward to find first chunk >= nb
        Chunk* victim = nullptr;
        while (p != sentinel) {
            size_t psize = p->chunk_size();
            if (psize >= nb.value) {
                if (!victim || psize < victim->chunk_size()) {
                    victim = p;
                    if (psize == nb.value) break;  // exact fit, best case
                }
            }
            p = p->bk;  // move towards larger sizes (sentinel->bk = smallest)
        }

        if (!victim) continue;

        size_t v_size = victim->chunk_size();
        bin.unlink(victim);
        if (bin.empty()) unmark_bin(UNSORTED_BIN_IDX + 1 + NSMALLBINS + i);

        // Split if remainder >= MINSIZE
        size_t remainder_size = v_size - nb.value;
        if (remainder_size >= MINSIZE) {
            // Set up the returned chunk
            ChunkFlag orig_flags = victim->flags();
            victim->set_head(nb, orig_flags);

            // Create remainder chunk
            Chunk* remainder = reinterpret_cast<Chunk*>(
                reinterpret_cast<uintptr_t>(victim) + nb.value);
            remainder->prev_size = nb.value;
            remainder->set_head(ChunkSize{remainder_size}, ChunkFlag::PREV_INUSE | orig_flags);
            remainder->set_foot(ChunkSize{remainder_size});

            // Put remainder in unsorted bin
            unsorted_bin.push_front(remainder);
            mark_bin(UNSORTED_BIN_IDX);

            return {victim, remainder};
        }

        // No split, use entire chunk
        // Adjust the next chunk's prev_size
        Chunk* next_chunk = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(victim) + v_size);
        // Not needed — the chunk is the same size, just marked in-use by caller

        return {victim, nullptr};
    }

    return {nullptr, nullptr};
}

bool BinManager::contains_free_chunk(Chunk* p) const noexcept {
    if (!p) return false;

    // Check all bins (brute force — for learning clarity)
    // Fastbins
    for (unsigned i = 0; i < NFASTBINS; ++i) {
        Chunk* cur = fastbins_[i].head.load(std::memory_order_acquire);
        while (cur) {
            if (cur == p) return true;
            cur = cur->fd;
        }
    }
    // Regular bins
    for (unsigned i = 1; i < N_BINS; ++i) {
        const BinList& bin = bins_[i];
        if (bin.empty()) continue;
        const Chunk* sentinel = bin.as_chunk();
        const Chunk* cur = bin.fd;
        while (cur && cur != sentinel) {
            if (cur == p) return true;
            cur = cur->fd;
        }
    }
    return false;
}

bool BinManager::unlink_free_chunk(Chunk* p) noexcept {
    // Try to find and unlink p from whichever bin it's in
    // Fastbins
    for (unsigned i = 0; i < NFASTBINS; ++i) {
        // Fastbins are singly-linked — need to find predecessor
        Chunk* prev = nullptr;
        Chunk* cur = fastbins_[i].head.load(std::memory_order_acquire);
        while (cur) {
            if (cur == p) {
                if (prev) prev->fd = cur->fd;
                else fastbins_[i].head.store(cur->fd, std::memory_order_release);
                return true;
            }
            prev = cur;
            cur = cur->fd;
        }
    }
    // Regular bins (doubly-linked, use unlink)
    for (unsigned i = 1; i < N_BINS; ++i) {
        BinList& bin = bins_[i];
        if (bin.empty()) continue;
        Chunk* sentinel = bin.as_chunk();
        Chunk* cur = bin.fd;
        while (cur && cur != sentinel) {
            if (cur == p) {
                bin.unlink(p);
                unmark_bin(i);
                return true;
            }
            cur = cur->fd;
        }
    }
    return false;
}

size_t BinManager::free_chunk_count() const noexcept {
    size_t total = 0;

    // Fastbins
    for (unsigned i = 0; i < NFASTBINS; ++i) {
        Chunk* cur = fastbins_[i].head.load(std::memory_order_acquire);
        while (cur) { total++; cur = cur->fd; }
    }
    // Regular bins
    for (unsigned i = 1; i < N_BINS; ++i) {
        const BinList& bin = bins_[i];
        if (bin.empty()) continue;
        const Chunk* sentinel = bin.as_chunk();
        const Chunk* cur = bin.fd;
        while (cur && cur != sentinel) { total++; cur = cur->fd; }
    }
    return total;
}

} // namespace ptmalloc
} // namespace my_ptmalloc
