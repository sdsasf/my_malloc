#pragma once
// Strong types and compile-time index computation
// Prevents mixing different categories of size/index values

#include "config.h"
#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {

// ─── Strong size types ───

struct UserSize {
    size_t value;
};

struct ChunkSize {
    size_t value;
};

// ─── Chunk flags ───

enum class ChunkFlag : size_t {
    NONE           = 0,
    PREV_INUSE     = PREV_INUSE_BIT,
    IS_MMAPPED     = IS_MMAPPED_BIT,
    NON_MAIN_ARENA = NON_MAIN_ARENA_BIT,
};

constexpr ChunkFlag operator|(ChunkFlag a, ChunkFlag b) noexcept {
    return static_cast<ChunkFlag>(static_cast<size_t>(a) | static_cast<size_t>(b));
}

constexpr ChunkFlag operator&(ChunkFlag a, ChunkFlag b) noexcept {
    return static_cast<ChunkFlag>(static_cast<size_t>(a) & static_cast<size_t>(b));
}

constexpr ChunkFlag operator~(ChunkFlag a) noexcept {
    return static_cast<ChunkFlag>(~static_cast<size_t>(a));
}

constexpr ChunkFlag& operator|=(ChunkFlag& a, ChunkFlag b) noexcept {
    return a = a | b;
}

constexpr bool has_flag(ChunkFlag flags, ChunkFlag bit) noexcept {
    return (static_cast<size_t>(flags) & static_cast<size_t>(bit)) != 0;
}

// ─── Strong bin index types ───

struct FastbinIdx {
    size_t value;
};

struct SmallbinIdx {
    size_t value;
};

struct LargebinIdx {
    size_t value;
};

struct TcacheIdx {
    size_t value;
};

// ─── Compile-time index computation ───

// Convert user request size to chunk size (includes header overhead)
[[nodiscard]] constexpr ChunkSize request2size(UserSize req) noexcept {
    // Account for size field (prev_size of allocated chunk is usable by prev chunk)
    // Round up to alignment, minimum MINSIZE
    size_t s = (req.value + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;
    if (s < MINSIZE) s = MINSIZE;
    return ChunkSize{s};
}

// Fastbin index: chunk_size / 16 on 64-bit
[[nodiscard]] constexpr FastbinIdx fastbin_index(ChunkSize cs) noexcept {
    return FastbinIdx{cs.value >> (SIZE_SZ == 8 ? 4 : 3)};
}

// Smallbin index: chunk_size / SMALLBIN_WIDTH
[[nodiscard]] constexpr SmallbinIdx smallbin_index(ChunkSize cs) noexcept {
    return SmallbinIdx{cs.value / SMALLBIN_WIDTH};
}

// Tcache index: (chunk_size - MINSIZE) / MALLOC_ALIGNMENT
[[nodiscard]] constexpr TcacheIdx csize2tidx(ChunkSize cs) noexcept {
    return TcacheIdx{(cs.value - MINSIZE) / MALLOC_ALIGNMENT};
}

// Reverse: tcache index to chunk size
[[nodiscard]] constexpr ChunkSize tidx2csize(TcacheIdx idx) noexcept {
    return ChunkSize{idx.value * MALLOC_ALIGNMENT + MINSIZE};
}

// Check if size falls in small bin range
[[nodiscard]] constexpr bool in_smallbin_range(ChunkSize cs) noexcept {
    return cs.value < MIN_LARGE_SIZE;
}

// Get large bin index for a given chunk size
[[nodiscard]] constexpr LargebinIdx largebin_index(ChunkSize cs) noexcept {
    size_t sz = cs.value;
    size_t idx = 0;
    if (sz >> 6 <= 4) {
        idx = 66 - 4 + (sz >> 6);          // bins 66-71: step 64
    } else if (sz >> 9 <= 4) {
        idx = 72 - 4 + (sz >> 9);          // bins 72-77: step 512
    } else if (sz >> 12 <= 4) {
        idx = 78 - 4 + (sz >> 12);         // bins 78-83: step 4096
    } else if (sz >> 15 <= 4) {
        idx = 84 - 4 + (sz >> 15);         // bins 84-89: step 32768
    } else if (sz >> 18 <= 4) {
        idx = 90 - 4 + (sz >> 18);         // bins 90-95: step 262144
    } else if (sz >> 21 <= 4) {
        idx = 96 - 4 + (sz >> 21);         // bins 96-101: step 2097152
    } else {
        idx = 102;                          // overflow bin
    }
    return LargebinIdx{idx};
}

// Check if size fits in tcache
[[nodiscard]] constexpr bool in_tcache_range(ChunkSize cs) noexcept {
    return cs.value <= tidx2csize(TcacheIdx{TCACHE_MAX_BINS - 1}).value;
}

// Binmap word/bit extraction
[[nodiscard]] constexpr size_t binmap_word(size_t bin_idx) noexcept {
    return bin_idx / (sizeof(unsigned int) * 8);
}

[[nodiscard]] constexpr unsigned int binmap_bit(size_t bin_idx) noexcept {
    return 1u << (bin_idx % (sizeof(unsigned int) * 8));
}

} // namespace my_ptmalloc
