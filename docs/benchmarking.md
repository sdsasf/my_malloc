# Benchmarking Guide

This project uses two benchmark layers:

1. Built-in configurable benchmarks through `bench_runner`.
2. External allocator benchmark suites, especially `mimalloc-bench`, through `LD_PRELOAD`.

The built-in runner is for fast iteration and regression checks. External suites are for stronger comparison against allocator research and production allocators.

## 1. Build

```bash
cmake -B build .
cmake --build build -j$(nproc)
```

The benchmark target is:

```bash
./build/bench_runner --help
```

## 2. Strategy Selection

Every built-in benchmark runs through the strategy API:

```bash
./build/bench_runner --strategy hybrid
./build/bench_runner --strategy ptmalloc
./build/bench_runner --strategy libc
./build/bench_runner --strategy plugin:./build/libcounting_malloc_strategy.so
```

Strategy meanings:

| Strategy | Meaning |
|---|---|
| `hybrid` | Small-object slab frontend plus ptmalloc-style fallback. |
| `ptmalloc` | Slab disabled; chunk/tcache/bin/arena backend only. |
| `libc` / `glibc` | System allocator baseline. |
| `plugin:path.so` | Custom external strategy. |

Validate custom strategies before benchmarking:

```bash
./build/allocator_validate --strategy plugin:./libyour_strategy.so
```

## 3. Profiles

Profiles are named groups of benchmark methods:

```bash
./build/bench_runner --profile smoke
./build/bench_runner --profile micro
./build/bench_runner --profile stress
./build/bench_runner --profile all
```

| Profile | Benchmarks | Purpose |
|---|---|---|
| `smoke` | `same_size_64`, `random` | Quick sanity check after edits. |
| `micro` | `same_size_64`, `same_size_256`, `batch`, `random` | Default fast comparison profile. |
| `stress` | `random`, `fragmentation`, `cross_thread_free` | Mixed-size, realloc, and ownership stress. |
| `all` | all built-in benchmarks | Broader local run before publishing results. |

## 4. Individual Benchmarks

You can choose exact benchmarks with repeated `--bench` flags:

```bash
./build/bench_runner --bench same_size --size 32
./build/bench_runner --bench same_size_64 --bench same_size_256
./build/bench_runner --bench fragmentation --bench cross_thread_free
```

Available built-in benchmarks:

| Benchmark | What it measures | Allocator behavior it stresses |
|---|---|---|
| `same_size` | Allocate/free one configurable size repeatedly. | Slab/tcache hot path, metadata overhead. |
| `same_size_64` | Fixed 64-byte hot path. | Small-object fast path. |
| `same_size_256` | Fixed 256-byte hot path. | Small-object fast path with larger class. |
| `batch` | Allocate a batch of objects, then free the whole batch. | Thread-local cache refill/drain and central cache behavior. |
| `random` | Random allocate/free/realloc over live slots. | General-purpose mixed workload, bin search, fragmentation side effects. |
| `fragmentation` | Realloc existing live objects across a size range. | In-place realloc, coalescing, split policy, RSS pressure. |
| `cross_thread_free` | Allocate in producer threads and free from different threads. | Thread-local cache ownership and remote-free weakness. |
| `latency_sample` | Same-size operations with sampled per-op timing in the result name. | Rough tail-latency smoke check. |

## 5. Parameters

The runner is deterministic by default and exposes workload parameters:

```bash
./build/bench_runner \
  --strategy hybrid \
  --bench random \
  --random-iters 1000000 \
  --slots 8192 \
  --min-size 16 \
  --max-size 16384 \
  --seed 7
```

Common options:

| Option | Applies to | Meaning |
|---|---|---|
| `--iters N` | `same_size`, `fragmentation`, `latency_sample` | Operation count. |
| `--random-iters N` | `random` | Operation count for random mixed workload. |
| `--size N` | `same_size`, `latency_sample` | Object size. |
| `--min-size N` / `--max-size N` | `random`, `fragmentation`, `cross_thread_free` | Size distribution range. |
| `--slots N` | `random`, `fragmentation` | Live pointer table size. |
| `--batch N` | `batch`, `cross_thread_free` | Batch size or per-thread object count. |
| `--rounds N` | `batch` | Number of batch rounds. |
| `--threads N` | `cross_thread_free` | Number of producer/consumer threads. |
| `--seed N` | random-like tests | Deterministic RNG seed. |

## 6. JSON Output

Use JSON lines for scripts:

```bash
./build/bench_runner --strategy hybrid --profile all --json
```

Example output:

```json
{"strategy":"hybrid","benchmark":"same_size_64","ops_per_sec":44943821,"ms":0.222,"peak_rss_kb":15872}
```

Each row includes:

- `strategy`;
- `benchmark`;
- `ops_per_sec`;
- elapsed `ms`;
- process peak RSS in KB.

Peak RSS is read from `getrusage(RUSAGE_SELF).ru_maxrss`. It is useful for relative comparison inside one run style, but it is not a complete fragmentation metric by itself.

## 7. Recommended Local Matrix

For routine development:

```bash
./build/allocator_validate --strategy hybrid
./build/allocator_validate --strategy ptmalloc
./build/bench_runner --strategy hybrid --profile smoke
./build/bench_runner --strategy ptmalloc --profile smoke
```

Before publishing a benchmark table:

```bash
for s in hybrid ptmalloc libc; do
  ./build/bench_runner --strategy "$s" --profile all --json
done
```

For adaptive policy experiments:

```bash
./build/bench_runner --strategy hybrid --bench random --bench fragmentation --json
./build/bench_runner --strategy hybrid --bench cross_thread_free --threads 8 --batch 10000 --json
./build/bench_runner --strategy hybrid --bench same_size --size 24 --json
./build/bench_runner --strategy hybrid --bench same_size --size 1024 --json
```

These runs expose different failure modes:

- small same-size tests reward slab/tcache hot paths;
- random tests expose bin and size-class behavior;
- fragmentation tests expose coalescing and realloc policy;
- cross-thread tests expose the lack of owner-thread remote-free lists.

## 8. External Benchmark Suites

### mimalloc-bench

`mimalloc-bench` is a practical allocator benchmark suite used by the mimalloc project. It collects classic allocator tests and application-like workloads including Larson-style server workloads, alloc-test, cache-scratch, xmalloc-test, cfrac, espresso, lean, and z3.

Repository:

```text
https://github.com/daanx/mimalloc-bench
```

Typical flow:

```bash
git clone https://github.com/daanx/mimalloc-bench
cd mimalloc-bench
./build-bench-env.sh all
cd out/bench
../../bench.sh sys larson
```

To test this project, preload the shared library into benchmarks that use the system malloc API:

```bash
LD_PRELOAD=/absolute/path/to/build/libmy_ptmalloc.so ../../bench.sh sys larson
```

When using external suites, compare against at least:

- system glibc malloc;
- jemalloc;
- tcmalloc;
- mimalloc;
- this project in default `hybrid` mode;
- this project in `MY_MALLOC_MODE=ptmalloc` mode.

### glibc benchtests

glibc has its own `benchtests`, including malloc/tcache hot-path tests. These are useful for studying ptmalloc-like behavior and tcache fast paths, but they are not a complete cross-allocator benchmark suite.

### Real Application Workloads

Allocator microbenchmarks are easy to overfit. Add real programs when possible:

- Redis or another server with many small objects;
- SQLite or RocksDB for mixed allocation lifetimes;
- clang or another compiler workload;
- Lua, Python, or Z3 for application-like allocation patterns;
- producer/consumer services to test cross-thread frees.

## 9. Reporting Rules

When documenting results, include:

- CPU model and core count;
- OS and kernel version;
- compiler and optimization flags;
- allocator commit hash;
- strategy and exact benchmark command;
- warm-up policy if any;
- ops/sec and elapsed time;
- peak RSS;
- notes about variance across multiple runs.

Do not compare one allocator's best profile against another allocator's different profile. Use the same command matrix for every strategy.

## 10. Current Limitations

The built-in runner is intentionally simple:

- it reports throughput and peak RSS, not full latency histograms;
- `latency_sample` is a rough smoke check, not a rigorous percentile benchmark;
- it does not yet report live bytes, mapped bytes, or internal fragmentation directly;
- it does not yet support aligned allocation or calloc-specific strategy methods;
- cross-thread free is synthetic and should be complemented by real producer/consumer workloads.

The next useful improvement is to add a structured result file with repeated runs and summary statistics:

```text
run_id, strategy, benchmark, params, iteration, ops_per_sec, p50, p99, peak_rss_kb
```

That would make performance regressions easier to track over time.
