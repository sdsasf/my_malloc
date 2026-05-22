#pragma once
// ptmalloc configuration constants (glibc malloc compatible).

#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {
namespace ptmalloc {

// ─── Fundamental constants (64-bit) ───
constexpr size_t SIZE_SZ           = sizeof(size_t);         // 8
constexpr size_t MALLOC_ALIGNMENT   = 2 * SIZE_SZ;           // 16
constexpr size_t MALLOC_ALIGN_MASK  = MALLOC_ALIGNMENT - 1;  // 15
constexpr size_t MINSIZE            = 4 * SIZE_SZ;           // 32 (min chunk size)
constexpr size_t CHUNK_HDR_SZ       = 2 * SIZE_SZ;           // 16 (prev_size + size)

// ─── Chunk flags (low 3 bits of size field) ───
constexpr size_t PREV_INUSE_BIT     = 0x1;
constexpr size_t IS_MMAPPED_BIT     = 0x2;
constexpr size_t NON_MAIN_ARENA_BIT = 0x4;

// ─── Fastbins ───
constexpr unsigned NFASTBINS        = 10;
constexpr size_t   MAX_FAST_SIZE    = 160;  // chunk size (usable ~152)

// ─── Tcache (glibc 2.26+) ───
constexpr unsigned TCACHE_MAX_BINS  = 64;
constexpr unsigned TCACHE_FILL_COUNT = 7;   // max entries per tcache bin

// ─── Smallbins ───
constexpr unsigned NSMALLBINS       = 64;
constexpr size_t   SMALLBIN_WIDTH   = 16;   // each bin covers 16 bytes of chunk size
constexpr size_t   MIN_LARGE_SIZE   = 1024; // chunk size threshold for large bins

// ─── Largebins ───
constexpr unsigned NLARGEBINS       = 63;
// Large bins are indexed 64..126 in the bin array.
// Bin 0 = unsorted, Bins 1..64 = smallbins, Bins 65..127 = largebins.

// ─── Bin array layout ───
constexpr unsigned BINMAPSIZE       = 4;     // bitmap words for bin non-empty tracking
constexpr unsigned N_BINS           = 128;   // total bins: 1(unsorted)+NSMALLBINS+NLARGEBINS-1

// Unsorted bin is bin index 1 in the full bin array.
// Smallbins: bin 2..65, Largebins: bin 65..127.

// ─── Mmap threshold ───
constexpr size_t DEFAULT_MMAP_THRESHOLD     = 128 * 1024;     // 128KB
constexpr size_t DEFAULT_MMAP_THRESHOLD_MAX = 512 * 1024;     // 512KB max
constexpr size_t DEFAULT_MMAP_THRESHOLD_MIN = 4096;           // 4KB min

// ─── Trim threshold ───
constexpr size_t DEFAULT_TRIM_THRESHOLD    = 128 * 1024;      // 128KB
constexpr size_t DEFAULT_TOP_PAD           = 0;               // no extra padding for top

// ─── Heap constants ───
constexpr size_t HEAP_MAX_SIZE    = 1024 * 1024;   // 1MB growth chunks
constexpr size_t HEAP_MIN_SIZE    = 64 * 1024;     // 64KB minimum

// ─── Thread constants ───
constexpr unsigned MAX_ARENAS      = 64;   // max number of non-main arenas

// ─── Size limits ───
constexpr size_t NSMALLBIN_MAX_SIZE = MIN_LARGE_SIZE;  // 1024

// ─── Type aliases ───
enum class ChunkFlag : size_t {
    PREV_INUSE     = PREV_INUSE_BIT,
    IS_MMAPPED     = IS_MMAPPED_BIT,
    NON_MAIN_ARENA = NON_MAIN_ARENA_BIT,
};

constexpr ChunkFlag operator|(ChunkFlag a, ChunkFlag b) noexcept {
    return static_cast<ChunkFlag>(static_cast<size_t>(a) | static_cast<size_t>(b));
}
constexpr size_t operator&(size_t v, ChunkFlag f) noexcept {
    return v & static_cast<size_t>(f);
}

struct ChunkSize { size_t value; };
struct UserSize  { size_t value; };
struct FastbinIdx { unsigned value; };
struct SmallbinIdx { unsigned value; };
struct LargebinIdx { unsigned value; };
struct TcacheIdx { unsigned value; };

} // namespace ptmalloc
} // namespace my_ptmalloc
