# my_ptmalloc

A C++17 reimplementation of glibc's ptmalloc2 memory allocator with tcache support. Drop-in replacement via `LD_PRELOAD` for benchmarking, educational inspection, and experimentation.

## Features

- **Tcache** -- 76 thread-local cached bins (64 small + 12 large), 16 entries per bin, with safe-linking (XOR pointer mangling) and double-free detection
- **Fastbins** -- 10 lock-free CAS bins for chunks up to 160 bytes
- **Small bins** -- 64 exact-fit FIFO bins (16-byte granularity)
- **Large bins** -- 62 sorted bins with best-fit search and binmap skipping for empty ranges
- **Unsorted bin** -- bounded staging scan, exact-match tcache refill, and safe last-remainder reuse
- **Multi-arena** -- cache-line-aligned arenas (`alignas(128)`) with per-thread arena creation/reuse and `HeapInfo` ownership lookup on free
- **Coalescing** -- forward + backward chunk merging with top-chunk absorption
- **Static thresholds** -- fixed mmap/trim thresholds via `mallopt` (adaptive policy exists but is not wired in)
- **Strong types** -- `ChunkSize`, `UserSize`, `FastbinIdx`, etc. with `constexpr` index computation
- **LD_PRELOAD hooks** -- full C-linkage `malloc`/`free`/`calloc`/`realloc`/`memalign`/`posix_memalign`/`aligned_alloc`/`mallopt`/`malloc_usable_size`
- **Bootstrap buffer** -- static per-thread buffers for pre-init allocations before `dlsym` resolves
- **Debug gating** -- hot-path metadata checks and stderr logging are compiled out unless `MY_PTMALLOC_ENABLE_DEBUG=1` is defined

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                    Public API                        │
│  my_malloc / my_free / my_calloc / my_realloc / ... │
├─────────────────────────────────────────────────────┤
│                 Allocation Pipeline                  │
│  TcacheAlloc → FastbinAlloc → SmallbinAlloc →       │
│  UnsortedAlloc → LargebinAlloc → TopChunkAlloc →    │
│  SysAlloc (mmap/new_heap)                           │
├──────────────┬──────────────┬───────────────────────┤
│   Tcache     │   FastBins   │   BinManager          │
│  (per-thread │  (lock-free  │  ┌─────────────────┐  │
│   76 bins)   │    CAS)      │  │ SmallBins (64)  │  │
│              │              │  │ LargeBins (62)  │  │
│              │              │  │ UnsortedBin (1) │  │
│              │              │  │ BinMap          │  │
│              │              │  └─────────────────┘  │
├──────────────┴──────────────┴───────────────────────┤
│                    Arena                             │
│  alignas(128), mutex, top chunk, last_remainder     │
├─────────────────────────────────────────────────────┤
│  ArenaManager ← ThreadRegistry ← SysMemory (mmap)  │
└─────────────────────────────────────────────────────┘
```

### Allocation Flow

1. **Tcache** -- thread-local, lock-free, O(1) for small sizes
2. **Fastbins** -- global, CAS lock-free, O(1) for tiny sizes
3. **Small bins** -- exact-fit FIFO under arena lock
4. **Unsorted bin** -- bounded scan, exact matches can refill current tcache class, other chunks are sorted into bins
5. **Large bins** -- binmap-assisted best-fit search in sorted bins
6. **Top chunk** -- carve from arena's top chunk
7. **System** -- `mmap` for large chunks, `new_heap` for arena growth

### Deallocation Flow

1. **Tcache** -- if in tcache range and not full, push to thread-local bin
2. **Consolidation** -- merge with adjacent free chunks (forward + backward)
3. **Top merge** -- if adjacent to top chunk, absorb into top
4. **Unsorted bin** -- place consolidated chunk for deferred sorting

## Build

```bash
cmake -B build .
cmake --build build -j$(nproc)
```

### Build Targets

| Target | Description |
|--------|-------------|
| `my_ptmalloc` | Shared library (`.so`) for `LD_PRELOAD` |
| `my_ptmalloc_static` | Static library for linking tests |
| `test_basic` | Unit tests (26 tests) |
| `test_tcache` | Tcache-specific tests |
| `test_stress` | Multi-threaded stress test |
| `test_perf` | Internal performance benchmark |
| `bench_my` | Comparison benchmark (my_ptmalloc) |
| `bench_sys` | Comparison benchmark (glibc) |
| `heap_inspect` | Heap state inspection tool |

### Run Tests

```bash
cd build && ctest --output-on-failure
```

## Usage

### As LD_PRELOAD Replacement

```bash
# Replace system malloc for any program
LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program

# With debug output
LD_PRELOAD=./build/libmy_ptmalloc.so MALLOC_TRACE=/tmp/trace.log ./your_program
```

### C++ API

```cpp
#include "my_ptmalloc/my_malloc.h"

void* p = my_ptmalloc::my_malloc(1024);
my_ptmalloc::my_free(p);

void* p2 = my_ptmalloc::my_calloc(10, 100);  // zero-initialized
void* p3 = my_ptmalloc::my_realloc(p2, 2000);
void* p4 = my_ptmalloc::my_memalign(64, 4096);  // aligned

size_t usable = my_ptmalloc::my_malloc_usable_size(p4);
```

### Runtime Tuning

```cpp
// Set mmap threshold (bytes above this use mmap instead of heap)
my_ptmalloc::my_mallopt(M_MMAP_THRESHOLD, 131072);

// Set trim threshold (release memory to OS when top chunk exceeds this)
my_ptmalloc::my_mallopt(M_TRIM_THRESHOLD, 128 * 1024);
```

## Benchmark Results

Tested on Linux x86_64, `-O2 -march=native`, on May 4, 2026. Each benchmark runs identical workloads against both allocators. Ratios are my_ptmalloc / glibc for throughput (higher is better).

### Throughput (ops/sec)

| Benchmark | glibc malloc | my_ptmalloc | Ratio |
|-----------|-------------|-------------|-------|
| Same-size 32B (10M alloc+free) | 74,037,422 | 54,417,130 | 0.73x |
| Same-size 64B (10M alloc+free) | 59,666,379 | 60,552,607 | 1.01x |
| Same-size 256B (10M alloc+free) | 35,982,169 | 30,009,090 | 0.83x |
| Batch 512x1000 (512k alloc+free) | 21,266,321 | 27,883,891 | 1.31x |
| Random alloc/free/realloc (200k) | 2,362,146 | 887,288 | 0.38x |
| Large 8-128KB (30k alloc+free) | 152,627 | 103,324 | 0.68x |
| Fragmentation (5k reallocs) | 41,166,834 | 3,004,867 | 0.07x |
| Multi-thread 4x50k (200k total) | 4,075,257 | 2,038,841 | 0.50x |

### Memory Footprint (Peak RSS)

| Benchmark | glibc malloc | my_ptmalloc | Overhead |
|-----------|-------------|-------------|----------|
| Random alloc/free | 16,896 KB | 19,328 KB | 1.1x |
| Same-size / batch | 16,896 KB | 19,456 KB | 1.2x |
| Large alloc | 20,032 KB | 25,128 KB | 1.3x |
| Fragmentation | 20,032 KB | 25,792 KB | 1.3x |
| Multi-thread | 100,996 KB | 116,160 KB | 1.2x |

### Analysis

- **Hot path (same-size)**: 64B is roughly at glibc parity in this run. 32B and 256B are slower than the previous run because the allocator now pays more correctness cost in shared bin/coalescing helpers.
- **Batch operations**: still faster than glibc because most short-lived chunks stay in tcache.
- **Random alloc/free/realloc**: still slower than glibc, but peak RSS improved after bin-aware coalescing and in-place realloc growth.
- **Large allocations**: throughput is below glibc, but RSS overhead dropped sharply because top-merge trimming and better coalescing reduce retained heap memory.
- **Multi-threaded**: lower than the previous benchmark run; arena distribution helps contention, but the benchmark still pays for simpler cross-thread arena reuse and coalescing policies.
- **Fragmentation microbench**: improved from the previous documented 582,909 ops/sec and 89MB+ RSS to 3,004,867 ops/sec and 25,792KB peak RSS. It remains far behind glibc because realloc and trimming are still much simpler.

### Optimization Notes

This version implements the planned hot-path optimizations:

- `my_malloc` tries tcache before calling `ArenaManager::get_arena`, so tcache hits avoid arena mutexes completely.
- `ArenaManager::get_arena` creates/reuses per-thread arenas up to `ncpus * ARENA_MULTIPLIER`; non-main heap regions are `HEAP_MAX_SIZE` aligned so `HeapInfo::arena_for_chunk` can map frees back to owners.
- `UnsortedBin::scan_and_sort` has a fixed scan budget, exact-match tcache refill, and avoids arbitrary first-fit splitting that caused fragmentation in random workloads.
- `LargeBins` uses `BinMap::find_first_from` to skip empty large-bin ranges before walking a sorted list.
- `BinManager::unlink_free_chunk` centralizes unlinking from unsorted, small, and large bins, so coalescing can safely merge chunks that have already been sorted out of unsorted.
- `my_realloc` can now grow in place by absorbing the top chunk or the next linked free chunk, splitting any usable remainder back to unsorted.
- Top-chunk merges call `systrim` using the configured threshold policy.
- Debug validation and `fprintf` calls in hot code are behind `MY_PTMALLOC_ENABLE_DEBUG`.

See [docs/allocator_design.md](docs/allocator_design.md) for a detailed implementation and design explanation.

## Project Structure

```
my_ptmalloc/
├── CMakeLists.txt
├── README.md
├── include/my_ptmalloc/
│   ├── config.h              # Platform constants (constexpr)
│   ├── types.h               # Strong types: ChunkSize, UserSize, BinIndex
│   ├── chunk.h               # Chunk struct with size/flag methods
│   ├── intrusive_list.h      # Doubly-linked circular list for bins
│   ├── fastbins.h            # Lock-free CAS fastbins
│   ├── small_bins.h          # 64 exact-fit FIFO bins
│   ├── large_bins.h          # 62 sorted bins with best-fit
│   ├── unsorted_bin.h        # Single staging bin
│   ├── bin_map.h             # Bitmap for bin occupancy
│   ├── bin_manager.h         # Facade composing all bin types
│   ├── tcache.h              # Thread-local cache with safe-linking
│   ├── arena.h               # Cache-line-aligned arena
│   ├── heap.h                # HeapInfo for non-main arenas
│   ├── sys_memory.h          # Abstract system memory source
│   ├── threshold.h           # Adaptive mmap/trim thresholds
│   ├── coalesce.h            # Configurable coalescing policy
│   ├── observer.h            # AllocObserver for stats/debug
│   ├── thread_registry.h     # Thread lifecycle management
│   ├── arena_manager.h       # Arena creation/selection
│   ├── alloc_pipeline.h      # Chain-of-responsibility allocation
│   ├── my_malloc.h           # Public C++ API
│   └── hooks.h               # LD_PRELOAD C linkage
├── src/
│   ├── init.cpp              # Global initialization
│   ├── chunk.cpp             # Chunk methods
│   ├── intrusive_list.cpp    # List operations
│   ├── fastbins.cpp          # CAS-based fastbin ops
│   ├── small_bins.cpp        # Small bin ops
│   ├── large_bins.cpp        # Large bin best-fit search
│   ├── unsorted_bin.cpp      # Unsorted bin scan + sort
│   ├── bin_map.cpp           # Bitmap scan
│   ├── bin_manager.cpp       # Bin facade
│   ├── tcache.cpp            # Tcache lifecycle
│   ├── arena.cpp             # Arena lifecycle
│   ├── heap.cpp              # Heap allocation (mmap-based)
│   ├── sys_memory.cpp        # MmapMemory implementation
│   ├── threshold.cpp         # Adaptive thresholds
│   ├── coalesce.cpp          # Coalescing policies
│   ├── observer.cpp          # Stats/debug observers
│   ├── thread_registry.cpp   # Thread management
│   ├── arena_manager.cpp     # Arena selection strategy
│   ├── alloc_pipeline.cpp    # Allocation strategy pipeline
│   ├── malloc_impl.cpp       # _int_malloc
│   ├── free_impl.cpp         # _int_free with consolidation
│   ├── realloc_impl.cpp      # _int_realloc
│   ├── consolidate.cpp       # malloc_consolidate + systrim
│   ├── hooks.cpp             # LD_PRELOAD interposition
│   └── my_malloc.cpp         # Public API wrappers
├── test/
│   ├── test_basic.cpp        # Unit tests
│   ├── test_tcache.cpp       # Tcache tests
│   ├── test_stress.cpp       # Multi-threaded stress
│   ├── test_perf.cpp         # Internal perf benchmark
│   └── bench_compare.cpp     # glibc vs my_ptmalloc comparison
└── tools/
    └── heap_inspect.cpp      # Heap state inspection
```

## Design Decisions

### Why C++ instead of C?

glibc's malloc.c is ~5700 lines of C with heavy macro usage. This reimplementation uses:
- Strong types to prevent size-class confusion at compile time
- RAII for arena locking (`ArenaGuard`)
- `constexpr` for index computation
- Classes for bin types with clear encapsulation
- `alignas` for cache-line isolation

### Why not restore fd_nextsize chain yet?

The `fd_nextsize`/`bk_nextsize` secondary chain in large bins is a performance optimization (O(1) skip to next size class). However, it's fundamentally fragile because:
- Chunk memory overlaps with user data when allocated
- `consolidate_and_free` merges chunks without updating large bin chains
- Stale pointers from previously-allocated chunks can cause infinite loops

Large bins now use binmap-assisted search over sorted fd/bk lists. This keeps the correctness of one linked-list representation while avoiding scans over empty large-bin ranges. Restoring `fd_nextsize` is still possible, but it should be done together with complete chain maintenance in split, unlink, and coalescing paths.

## Limitations

- `systrim` is triggered on top merges, but not yet on every possible heap-growth/free path
- Thread-local tcache is flushed on thread exit, but cleanup is not automatically wired into the OS thread destructor path yet
- No `malloc_info` / `malloc_stats` implementation
- No `mallopt` hooks for all glibc tuning parameters
- `realloc` can grow into the top chunk or next linked free chunk, but still lacks several glibc-grade cases such as mmap remap and broader shrink/split heuristics

## License

Educational/research project. See source files for details.
