# my_malloc

`my_malloc` is a C++17 learning-oriented memory allocator. It can be used as an `LD_PRELOAD` malloc replacement, and it also provides a strategy/benchmark lab for comparing allocator designs.

The project tries to reproduce the core ideas of industrial allocators without fully cloning glibc malloc, jemalloc, tcmalloc, or mimalloc. Those implementations are the teaching allocator lab. The separate `adaptive` allocator is the project's experimental design: it owns its metadata and telemetry, and can switch among adaptive-specific internal strategies at runtime.

## What This Project Implements

The project is organized around four parts:

1. **Allocator lab**: simplified implementations of classic industrial allocator ideas for learning and comparison.
2. **Adaptive allocator**: an independent experimental allocator with adaptive-owned metadata, telemetry, policy, and internal strategies.
3. **Common harness**: validation, built-in benchmarks, optional external benchmarks, and LD_PRELOAD smoke tests.
4. **Technical docs**: design documents for each allocator, including architecture, data structures, simplifications, and benchmark guidance.

Runtime-selectable strategies:

| Strategy | Category | Core design |
|---|---|---|
| `hybrid` | Teaching allocator | 16B size-class slab frontend for small objects plus ptmalloc-style fallback. |
| `ptmalloc` | Teaching allocator | Boundary-tag chunks, tcache, fastbins, small/unsorted/large bins, arenas, top chunk, mmap. |
| `tcmalloc_like` | Teaching allocator | Size classes, thread caches, central free lists, batch refill/drain, 64KB spans. |
| `jemalloc_like` | Teaching allocator | Arenas, per-thread tcache, size-class runs, arena-local refill. |
| `mimalloc_like` | Teaching allocator | Per-thread heaps, page ownership, local free lists, remote-free queues. |
| `adaptive` | Experimental allocator | Adaptive-owned metadata, 64KB small pages, 256KB medium spans, direct-mmap large blocks, adaptive-internal policy. |
| `adaptive_demo` / `demo_all` | Teaching demo | Legacy dispatcher across teaching allocators for policy demonstration and mixed-ownership stress tests. |
| `libc` / `glibc` | Baseline | System allocator for comparison in tools. |

Main features:

| Area | Implemented feature |
|---|---|
| Public API | `malloc`, `free`, `calloc`, `realloc`, `memalign`, `posix_memalign`, `aligned_alloc`, `mallopt`, `malloc_usable_size` |
| Drop-in usage | `LD_PRELOAD=./build/libmy_ptmalloc.so ./program` |
| Runtime modes | `hybrid`, `ptmalloc`, `tcmalloc_like`, `jemalloc_like`, `mimalloc_like`, `adaptive`, `adaptive_demo` |
| Learning tools | heap inspector, statistics, trace option, strategy API, plugin example |
| Benchmarks | built-in configurable benchmark runner plus external `mimalloc-bench`, Redis, and real-application smoke hooks |

## Architecture At A Glance

```mermaid
flowchart TB
    Project["my_malloc"]
    Lab["Allocator lab<br/>industrial ideas for learning"]
    Adaptive["Independent adaptive allocator<br/>experimental backend"]
    Harness["Common validation + benchmark harness"]
    Docs["Per-allocator technical docs"]

    Project --> Lab
    Project --> Adaptive
    Project --> Harness
    Project --> Docs

    Lab --> Hybrid["hybrid"]
    Lab --> PT["ptmalloc"]
    Lab --> TC["tcmalloc_like"]
    Lab --> JE["jemalloc_like"]
    Lab --> MI["mimalloc_like"]

    Adaptive --> Small["SmallObjectStrategy<br/>16B classes / 64KB pages"]
    Adaptive --> Medium["MediumObjectStrategy<br/>1KiB classes / 256KB spans"]
    Adaptive --> Large["LargeObjectStrategy<br/>direct mmap"]
```

Public allocation calls still use one API surface. New allocations choose a mode through `MY_MALLOC_MODE`, while `free`/`realloc` route by pointer ownership metadata so objects return to the allocator that created them.

For the detailed system architecture, read [docs/system_architecture.md](docs/system_architecture.md).

## Build

```bash
cmake -B build .
cmake --build build -j$(nproc)
```

Important build targets:

| Target | Purpose |
|---|---|
| `my_ptmalloc` | Shared library for `LD_PRELOAD` |
| `my_ptmalloc_static` | Static library used by tests/tools |
| `test_basic` | Basic correctness tests |
| `test_tcache` | Tcache-specific tests |
| `test_stress` | Multi-threaded stress test |
| `allocator_validate` | Validates built-in or plugin strategies |
| `bench_runner` | Configurable benchmark runner |
| `heap_inspect` | Heap inspection tool |
| `example_counting_strategy` | Example external allocator strategy plugin |

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

Validate allocator strategies:

```bash
./build/allocator_validate --strategy hybrid
./build/allocator_validate --strategy ptmalloc
./build/allocator_validate --strategy tcmalloc_like
./build/allocator_validate --strategy jemalloc_like
./build/allocator_validate --strategy mimalloc_like
./build/allocator_validate --strategy adaptive
./build/allocator_validate --strategy libc
```

Run with `LD_PRELOAD`:

```bash
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=hybrid ./build/test_basic
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=ptmalloc ./build/test_basic
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=tcmalloc_like ./build/test_basic
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=jemalloc_like ./build/test_basic
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=mimalloc_like ./build/test_basic
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=adaptive ./build/test_basic
```

## Use As A Drop-in Allocator

```bash
LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
```

Switch allocator mode:

```bash
MY_MALLOC_MODE=hybrid   LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_MODE=ptmalloc LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_MODE=tcmalloc_like LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_MODE=jemalloc_like LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_MODE=mimalloc_like LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_MODE=adaptive LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_MODE=adaptive MY_MALLOC_ADAPTIVE_POLICY=ucb1 LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_MODE=adaptive MY_MALLOC_ADAPTIVE_POLICY=thompson_sampling LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_MODE=adaptive_demo MY_MALLOC_ADAPTIVE_POLICY=bandit LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
```

`MY_MALLOC_MODE=adaptive` is a standalone adaptive allocator backend. Its policy switches only adaptive-internal strategies and does not default to routing through the teaching allocators. Baseline policies include `heuristic`, `fixed:small`, `fixed:medium`, `fixed:large`, and `round_robin`; telemetry-driven policies include `epsilon_greedy`, `ucb1`, and `thompson_sampling`. The old demonstration behavior that dispatches across teaching allocators is available as `adaptive_demo` / `demo_all`.

Enable optional observability:

```bash
MY_MALLOC_STATS=1 LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
MY_MALLOC_TRACE=1 LD_PRELOAD=./build/libmy_ptmalloc.so ./your_program
```

## Use From C++

```cpp
#include "my_ptmalloc/my_malloc.h"

void* p = my_ptmalloc::my_malloc(1024);
p = my_ptmalloc::my_realloc(p, 2048);
size_t usable = my_ptmalloc::my_malloc_usable_size(p);
my_ptmalloc::my_free(p);
```

Aligned allocation:

```cpp
void* p = my_ptmalloc::my_memalign(64, 4096);
my_ptmalloc::my_free(p);
```

## Benchmark

Built-in benchmarks:

```bash
./build/bench_runner --strategy hybrid --profile smoke
./build/bench_runner --strategy hybrid --profile micro
./build/bench_runner --strategy hybrid --profile stress --json
./build/bench_runner --strategy ptmalloc --profile all --json
./build/bench_runner --strategy tcmalloc_like --profile all --json
./build/bench_runner --strategy jemalloc_like --profile all --json
./build/bench_runner --strategy mimalloc_like --profile all --json
./build/bench_runner --strategy adaptive --profile all --json
./build/bench_runner --strategy libc --profile all --json
```

Run individual workloads:

```bash
./build/bench_runner --strategy hybrid --bench same_size --size 64 --iters 1000000
./build/bench_runner --strategy hybrid --bench random --random-iters 500000 --slots 8192
./build/bench_runner --strategy hybrid --bench fragmentation --iters 100000 --min-size 16 --max-size 16384
./build/bench_runner --strategy hybrid --bench cross_thread_free --threads 8 --batch 10000
```

External benchmarks:

```bash
scripts/external_bench.sh setup-mimalloc-bench
scripts/external_bench.sh build-mimalloc-bench bench
scripts/external_bench.sh build-mimalloc-bench redis
scripts/external_bench.sh run-mimalloc-bench rptest larson alloc-test glibc-thread
scripts/external_bench.sh run-redis
scripts/external_bench.sh run-real-apps
```

Full benchmark instructions and current results:

| Document | Contents |
|---|---|
| [docs/benchmarking.md](docs/benchmarking.md) | Built-in profiles, parameters, JSON output, external benchmark commands |
| [docs/external_benchmark_results.md](docs/external_benchmark_results.md) | Latest external run notes, Redis results, skipped tests, known environment blockers |

## Documentation Map

| Document | Read this for |
|---|---|
| [docs/system_architecture.md](docs/system_architecture.md) | Project architecture: teaching allocator lab, independent adaptive allocator, ownership, validation, and benchmark harness |
| [docs/ptmalloc_design.md](docs/ptmalloc_design.md) | ptmalloc-style allocator: chunk/tcache/bin/arena implementation and simplifications |
| [docs/tcmalloc_design.md](docs/tcmalloc_design.md) | tcmalloc-like allocator: size classes, thread cache, central lists, spans |
| [docs/jemalloc_design.md](docs/jemalloc_design.md) | jemalloc-like allocator: arenas, runs, tcache |
| [docs/mimalloc_design.md](docs/mimalloc_design.md) | mimalloc-like allocator: per-thread heaps, page ownership, remote-free queues |
| [docs/adaptive_allocator.md](docs/adaptive_allocator.md) | Independent adaptive backend: metadata, ownership, internal strategies, baseline policies, and future ML/RL hooks |
| [docs/allocator_lab.md](docs/allocator_lab.md) | Custom allocator strategy/plugin API and validation workflow |
| [docs/benchmarking.md](docs/benchmarking.md) | Benchmark methodology, commands, smoke results |
| [docs/external_benchmark_results.md](docs/external_benchmark_results.md) | External benchmark run notes and environment blockers |

## Current Performance Snapshot

The latest local benchmark run was performed on May 5, 2026 with `bench_runner`. It compares the teaching allocator implementations, the self-developed `adaptive` allocator, and `libc` as a baseline.

Micro profile highlights:

| Strategy | `same_size_64` ops/sec | `batch` ops/sec | `random` ops/sec |
|---|---:|---:|---:|
| `libc` | 12,822,325 | 7,127,544 | 1,046,455 |
| `hybrid` | 27,823,527 | 10,936,918 | 1,102,044 |
| `ptmalloc` | 26,197,541 | 11,054,527 | 1,157,274 |
| `tcmalloc_like` | 26,748,702 | 3,796,048 | 759,696 |
| `jemalloc_like` | 18,887,703 | 8,979,967 | 816,400 |
| `mimalloc_like` | 23,087,757 | 9,218,316 | 947,105 |
| `adaptive` | 23,284,677 | 3,073,215 | 827,614 |

Adaptive policy highlights from the same run:

| Adaptive policy | `same_size_64` ops/sec | `batch` ops/sec | `random` ops/sec |
|---|---:|---:|---:|
| `heuristic` | 8,185,376 | 8,230,347 | 844,231 |
| `epsilon_greedy` | 19,812,613 | 8,217,441 | 923,744 |
| `ucb1` | 22,236,701 | 9,492,521 | 922,979 |
| `thompson_sampling` | 18,707,184 | 2,483,404 | 796,735 |
| `round_robin` | 22,045,714 | 10,005,564 | 1,071,706 |

The results are useful for learning because they expose concrete design tradeoffs:

- `hybrid` and `ptmalloc` are strong on this local micro matrix;
- the teaching allocators can beat `libc` on narrow microbenchmarks, which does not imply production superiority;
- `adaptive` now runs as an independent allocator, but still pays overhead from telemetry, ownership registry, simple locked pools, and no page/span release;
- `ucb1` is a stronger adaptive baseline than plain heuristic in this run;
- broader LD_PRELOAD and external workload results are still needed before making general claims.

## Current Limitations

- Empty slabs are cached but not yet returned to the OS.
- Cross-thread slab frees do not yet use owner-thread remote-free queues.
- The size-class table is simple 16-byte spacing, not a production-tuned table.
- Large allocation and extent management are simpler than jemalloc/tcmalloc/mimalloc.
- The adaptive backend now has dedicated small pages, medium spans, and telemetry-driven bandit policies, but it still uses simple locked pools, uniform size classes, and no empty page/span release. Full contextual ML/RL policy work is a future extension over adaptive-internal signals.
- External benchmark coverage depends on local tools such as Redis, glibc benchtests, SQLite, clang, Z3, jemalloc, tcmalloc, and mimalloc.
