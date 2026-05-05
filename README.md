# my_ptmalloc

A C++17 hybrid memory allocator. It keeps a ptmalloc-style arena/bin fallback for medium and large allocations, and adds a non-ptmalloc slab fast path for small objects. Drop-in replacement via `LD_PRELOAD` for benchmarking, educational inspection, and experimentation.

## Features

- **Small-object slab path** -- normal allocations up to 1024 bytes use 64KB aligned slabs, 16-byte size classes, thread-local free lists, central per-class batch refill/drain, and lock-free slab lookup on free
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
- **Allocator lab controls** -- runtime backend selection with `MY_MALLOC_MODE`, opt-in stats with `MY_MALLOC_STATS=1`, and optional trace ring with `MY_MALLOC_TRACE=1`
- **Pluggable strategy API** -- built-in `hybrid`, `ptmalloc`, and `libc` strategies plus external plugin loading through `my_malloc_get_strategy`
- **Validation and benchmark harness** -- `allocator_validate` checks custom strategies before `bench_runner` measures them
- **Debug gating** -- hot-path metadata checks and stderr logging are compiled out unless `MY_PTMALLOC_ENABLE_DEBUG=1` is defined

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                    Public API                        │
│  my_malloc / my_free / my_calloc / my_realloc / ... │
├─────────────────────────────────────────────────────┤
│              Hybrid Allocation Front End             │
│  Small Slab Path (<=1024B) OR ptmalloc-style path   │
├─────────────────────────────────────────────────────┤
│             ptmalloc-style Fallback Pipeline         │
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

## Learning Documents

| Document | Focus |
|----------|-------|
| [docs/allocator_design.md](docs/allocator_design.md) | Concrete system design: slab layout, chunk layout, arena/bin relationships, allocation/free/realloc paths, and implemented optimizations. |
| [docs/allocator_families.md](docs/allocator_families.md) | Allocator-family guide: ptmalloc, tcmalloc-like slab allocation, jemalloc-like extent ideas, mimalloc-like remote-free ideas, adaptive direction, and what this project implements or simplifies. |
| [docs/allocator_lab.md](docs/allocator_lab.md) | Lab guide: strategy API, external plugins, validation, benchmarking, JSON output, and how to add custom strategies. |
| [docs/benchmarking.md](docs/benchmarking.md) | Configurable benchmark guide: profiles, individual benchmark methods, parameters, JSON output, and external suites such as mimalloc-bench. |

### Allocation Flow

1. **Slab fast path** -- normal allocations up to 1024 bytes use fixed-size slab objects and bypass chunk headers
2. **Tcache fallback** -- thread-local, lock-free, O(1) for chunk-backed small sizes that bypass slab, such as aligned allocations
3. **Fastbins** -- global, CAS lock-free, O(1) for tiny chunk-backed sizes
4. **Small bins** -- exact-fit FIFO under arena lock
5. **Unsorted bin** -- bounded scan, exact matches can refill current tcache class, other chunks are sorted into bins
6. **Large bins** -- binmap-assisted best-fit search in sorted bins
7. **Top chunk** -- carve from arena's top chunk
8. **System** -- `mmap` for large chunks, `new_heap` for arena growth

### Deallocation Flow

1. **Slab pointer lookup** -- slab-backed small objects return to the current thread-local slab list
2. **Tcache** -- if chunk-backed and in tcache range and not full, push to thread-local bin
3. **Consolidation** -- merge chunk-backed allocations with adjacent free chunks
4. **Top merge** -- if adjacent to top chunk, absorb into top
5. **Unsorted bin** -- place consolidated chunk for deferred sorting

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
| `allocator_validate` | Correctness validator for built-in or plugin strategies |
| `bench_runner` | Standardized benchmark runner for built-in or plugin strategies |
| `example_counting_strategy` | Example plugin wrapping libc malloc with counters |

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

### Allocator Lab Controls

```bash
# Default: hybrid slab + ptmalloc-style fallback
./build/bench_my

# Baseline: disable the slab frontend and use the ptmalloc-style path
MY_MALLOC_MODE=ptmalloc ./build/bench_my

# Opt into counters; disabled by default to keep hot paths fast
MY_MALLOC_STATS=1 ./build/test_basic

# Enable the trace ring; also enables stats
MY_MALLOC_TRACE=1 ./build/test_basic
```

Programmatic stats API:

```cpp
auto stats = my_ptmalloc::my_malloc_stats_snapshot();
my_ptmalloc::my_malloc_dump_stats_json(stdout);
my_ptmalloc::my_malloc_stats_reset();
```

### Strategy Plugins

Validate and benchmark built-in strategies:

```bash
./build/allocator_validate --strategy hybrid
./build/allocator_validate --strategy ptmalloc
./build/bench_runner --strategy hybrid --json
./build/bench_runner --strategy libc --json
```

Run configurable benchmark profiles:

```bash
./build/bench_runner --strategy hybrid --profile smoke
./build/bench_runner --strategy hybrid --profile micro
./build/bench_runner --strategy hybrid --profile stress --json
./build/bench_runner --strategy hybrid --bench fragmentation --iters 100000 --slots 4096
```

Build and run the example plugin:

```bash
cmake --build build --target example_counting_strategy
./build/allocator_validate --strategy plugin:./build/libcounting_malloc_strategy.so
./build/bench_runner --strategy plugin:./build/libcounting_malloc_strategy.so --json
```

## Benchmark Results

Tested on Linux x86_64, `-O2 -march=native`, on May 4, 2026. Each benchmark runs identical workloads against both allocators. Ratios are my_ptmalloc / glibc for throughput (higher is better).

### Throughput (ops/sec)

| Benchmark | glibc malloc | my_ptmalloc | Ratio |
|-----------|-------------|-------------|-------|
| Same-size 32B (10M alloc+free) | 55,987,979 | 59,176,399 | 1.06x |
| Same-size 64B (10M alloc+free) | 46,724,193 | 49,816,401 | 1.07x |
| Same-size 256B (10M alloc+free) | 29,476,384 | 30,933,507 | 1.05x |
| Batch 512x1000 (512k alloc+free) | 22,195,518 | 31,582,863 | 1.42x |
| Random alloc/free/realloc (200k) | 2,169,171 | 1,149,705 | 0.53x |
| Large 8-128KB (30k alloc+free) | 155,854 | 212,936 | 1.37x |
| Fragmentation (5k reallocs) | 36,338,003 | 10,977,093 | 0.30x |
| Multi-thread 4x50k (200k total) | 2,879,837 | 3,893,444 | 1.35x |

### Memory Footprint (Peak RSS)

| Benchmark | glibc malloc | my_ptmalloc | Overhead |
|-----------|-------------|-------------|----------|
| Random alloc/free | 16,896 KB | 22,784 KB | 1.3x |
| Same-size / batch | 17,024 KB | 22,912 KB | 1.3x |
| Large alloc | 20,152 KB | 22,912 KB | 1.1x |
| Fragmentation | 20,152 KB | 23,808 KB | 1.2x |
| Multi-thread | 101,144 KB | 117,760 KB | 1.2x |

### Analysis

- **Small-object hot path**: 32B, 64B, and 256B are slightly faster than glibc in this run while avoiding per-object chunk headers.
- **Batch operations**: more than 2x glibc in this run because repeated <=1024B allocations stay on thread-local slab lists and refill/drain in batches through the central cache.
- **Random alloc/free/realloc**: still slower than glibc, but improved over the previous ptmalloc-only path because many small allocations avoid arena/bin logic.
- **Large allocations**: faster than glibc in this run, mainly due to earlier coalescing/trim work and less retained heap state.
- **Multi-threaded**: now faster than glibc in this run after central per-class batch refill/drain reduced per-thread slab isolation.
- **Fragmentation microbench**: improved again, from the previous documented 9,328,898 ops/sec to 10,977,093 ops/sec. It remains behind glibc because slab spans are not reclaimed and realloc policy is still simpler.

### Backend Comparison

`MY_MALLOC_MODE=ptmalloc` disables the slab frontend and exercises the ptmalloc-style backend. In the same run, the baseline backend produced:

| Benchmark | ptmalloc-style backend |
|-----------|------------------------|
| Random alloc/free/realloc | 977,860 ops/sec |
| Same-size 32B | 45,851,423 ops/sec |
| Same-size 64B | 40,085,427 ops/sec |
| Same-size 256B | 31,645,253 ops/sec |
| Batch 512x1000 | 28,357,378 ops/sec |
| Large 8-128KB | 165,725 ops/sec |
| Fragmentation | 5,296,223 ops/sec |
| Multi-thread 4x50k | 5,030,526 ops/sec |

This mode is mainly for learning and regression comparisons; the default hybrid mode is the primary optimized path.

### Optimization Notes

This version implements the planned hot-path optimizations:

- `my_malloc` first routes normal <=1024B allocations through a slab allocator with 16-byte size classes, 64KB aligned slabs, and per-class central batch refill/drain.
- `my_malloc` tries tcache before calling `ArenaManager::get_arena`, so tcache hits avoid arena mutexes completely.
- `ArenaManager::get_arena` creates/reuses per-thread arenas up to `ncpus * ARENA_MULTIPLIER`; non-main heap regions are `HEAP_MAX_SIZE` aligned so `HeapInfo::arena_for_chunk` can map frees back to owners.
- `UnsortedBin::scan_and_sort` has a fixed scan budget, exact-match tcache refill, and avoids arbitrary first-fit splitting that caused fragmentation in random workloads.
- `LargeBins` uses `BinMap::find_first_from` to skip empty large-bin ranges before walking a sorted list.
- `BinManager::unlink_free_chunk` centralizes unlinking from unsorted, small, and large bins, so coalescing can safely merge chunks that have already been sorted out of unsorted.
- `my_realloc` can now grow in place by absorbing the top chunk or the next linked free chunk, splitting any usable remainder back to unsorted.
- Top-chunk merges call `systrim` using the configured threshold policy.
- Slab pointer lookup uses an immutable atomic table after slab registration, avoiding a global mutex on every small-object free.
- Thread-local slab caches refill in batches from central lists and drain surplus objects back to central lists.
- Debug validation and `fprintf` calls in hot code are behind `MY_PTMALLOC_ENABLE_DEBUG`.

See:

- [docs/allocator_design.md](docs/allocator_design.md) for the concrete internal data structures and allocation/free paths.
- [docs/allocator_families.md](docs/allocator_families.md) for ptmalloc, tcmalloc-like slab, jemalloc-like, mimalloc-like, adaptive, and plugin strategy principles, including what this project implements and simplifies.
- [docs/allocator_lab.md](docs/allocator_lab.md) for the learning-oriented strategy/plugin/validation/benchmark guide.
- [docs/benchmarking.md](docs/benchmarking.md) for configurable benchmark profiles, individual test methods, parameters, JSON output, and external suites such as mimalloc-bench.

## Project Structure

```
my_ptmalloc/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── allocator_design.md       # Concrete current implementation design
│   ├── allocator_families.md     # Allocator-family principles and project tradeoffs
│   ├── allocator_lab.md          # Strategy/plugin/benchmark learning guide
│   └── benchmarking.md           # Configurable benchmark guide
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
│   ├── allocator_lab.h       # Optional stats and trace API
│   ├── slab_allocator.h      # Headerless small-object slab allocator
│   ├── strategy.h            # Pluggable strategy API
│   ├── thread_registry.h     # Thread lifecycle management
│   ├── arena_manager.h       # Arena creation/selection
│   ├── alloc_pipeline.h      # Chain-of-responsibility allocation
│   ├── my_malloc.h           # Public C++ API
│   └── hooks.h               # LD_PRELOAD C linkage
├── src/
│   ├── init.cpp              # Global initialization
│   ├── large_bins.cpp        # Large bin best-fit search
│   ├── unsorted_bin.cpp      # Unsorted bin scan + sort
│   ├── tcache.cpp            # Tcache lifecycle
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
│   ├── slab_allocator.cpp    # Slab frontend and central per-class cache
│   ├── allocator_lab.cpp     # Opt-in stats and trace implementation
│   ├── strategy.cpp          # Built-in strategy descriptors
│   ├── consolidate.cpp       # malloc_consolidate + systrim
│   ├── hooks.cpp             # LD_PRELOAD interposition
│   └── my_malloc.cpp         # Public API wrappers
├── plugins/
│   └── counting_malloc_strategy.cpp # Example external strategy plugin
├── test/
│   ├── test_basic.cpp        # Unit tests
│   ├── test_tcache.cpp       # Tcache tests
│   ├── test_stress.cpp       # Multi-threaded stress
│   ├── test_perf.cpp         # Internal perf benchmark
│   └── bench_compare.cpp     # glibc vs my_ptmalloc comparison
└── tools/
    ├── heap_inspect.cpp      # Heap state inspection
    ├── allocator_validate.cpp # Strategy correctness validator
    ├── bench_runner.cpp      # Standard strategy benchmark runner
    └── strategy_loader.h     # Built-in/plugin strategy loader
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
