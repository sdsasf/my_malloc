#pragma once
// Platform constants for ptmalloc reimplementation
// All values are constexpr for compile-time evaluation

#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {

// Hot-path diagnostics are compiled out by default. Define
// MY_PTMALLOC_ENABLE_DEBUG=1 to keep defensive metadata checks and stderr logs.
#ifndef MY_PTMALLOC_ENABLE_DEBUG
#define MY_PTMALLOC_ENABLE_DEBUG 0
#endif

#if MY_PTMALLOC_ENABLE_DEBUG
#define MY_PTMALLOC_DEBUG 1
#else
#define MY_PTMALLOC_DEBUG 0
#endif

// ─── Fundamental sizes ───
constexpr size_t SIZE_SZ           = sizeof(size_t);           // 8 on 64-bit
constexpr size_t MALLOC_ALIGNMENT  = 2 * SIZE_SZ;              // 16
constexpr size_t MALLOC_ALIGN_MASK = MALLOC_ALIGNMENT - 1;
// In ptmalloc, user data starts at chunk + 2*SIZE_SZ (glibc's chunk2mem macro)
// BUT the prev_size field of the next chunk is the LAST SIZE_SZ bytes of the
// current chunk's user data. So the per-chunk overhead is effectively SIZE_SZ.
// chunk2mem(p) = p + 2*SIZE_SZ, mem2chunk(mem) = mem - 2*SIZE_SZ
// However, the usable size of a chunk is chunk_size - SIZE_SZ (not - 2*SIZE_SZ)
// because the last SIZE_SZ bytes (next chunk's prev_size) are usable for data.
constexpr size_t CHUNK_HDR_SZ      = 2 * SIZE_SZ;              // 16 (prev_size + size)
constexpr size_t CHUNK_OVERHEAD    = SIZE_SZ;                   // 8 effective overhead per chunk

// Minimum chunk size: 4 * SIZE_SZ = 32 bytes on 64-bit
// This ensures fd_nextsize/bk_nextsize pointers fit in a free chunk
constexpr size_t MIN_CHUNK_SIZE    = 4 * SIZE_SZ;
constexpr size_t MINSIZE           = (MIN_CHUNK_SIZE + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;

// ─── Bin counts ───
constexpr size_t NBINS       = 128;       // Total bin slots (1 unsorted + 64 small + 62 large + 1 unused)
constexpr size_t NSMALLBINS  = 64;
constexpr size_t SMALLBIN_WIDTH = MALLOC_ALIGNMENT;   // 16 bytes per small bin on 64-bit
constexpr size_t MIN_LARGE_SIZE = NSMALLBINS * SMALLBIN_WIDTH;  // 1024 bytes

// Binmap: 128 bits, stored as array of unsigned int
constexpr size_t BINMAPSIZE  = NBINS / (sizeof(unsigned int) * 8);  // 4 on 64-bit (128/32)

// ─── Fastbins ───
constexpr size_t MAX_FAST_SIZE = (80 * SIZE_SZ) / 4;  // 160 bytes on 64-bit
constexpr size_t NFASTBINS     = 10;  // fastbin_index(request2size(MAX_FAST_SIZE)) + 1
constexpr size_t FASTBIN_CONSOLIDATION_THRESHOLD = 65536;

// ─── Tcache ───
constexpr size_t TCACHE_SMALL_BINS = 64;
constexpr size_t TCACHE_LARGE_BINS = 12;
constexpr size_t TCACHE_MAX_BINS   = TCACHE_SMALL_BINS + TCACHE_LARGE_BINS;  // 76
constexpr size_t TCACHE_FILL_COUNT = 16;  // Max chunks per tcache bin
constexpr size_t UNSORTED_SCAN_LIMIT = 32; // Bound one malloc's unsorted work

// ─── Heap ───
constexpr size_t HEAP_MAX_SIZE     = 1024 * 1024;  // 1MB per heap region

// ─── Thresholds (defaults) ───
constexpr size_t DEFAULT_TRIM_THRESHOLD  = 128 * 1024;   // 128KB
constexpr size_t DEFAULT_MMAP_THRESHOLD  = 128 * 1024;   // 128KB
constexpr size_t DEFAULT_TOP_PAD         = 0;
constexpr size_t DEFAULT_N_MMAPS_MAX     = 65536;

// ─── Arena limits ───
constexpr int ARENA_MULTIPLIER = 8;  // On 64-bit: n_cpus * 8

// ─── Chunk flag bit positions ───
constexpr size_t PREV_INUSE_BIT     = 1;
constexpr size_t IS_MMAPPED_BIT     = 2;
constexpr size_t NON_MAIN_ARENA_BIT = 4;
constexpr size_t SIZE_BITS          = 7;  // All 3 flag bits

// ─── Arena flags ───
constexpr int FASTCHUNKS_BIT        = 1;

} // namespace my_ptmalloc
