# Benchmarking And Performance

This document explains how to test allocator correctness and performance. It covers built-in benchmarks, external benchmark suites, Redis, real-application smoke tests, and reporting rules.

## 1. Benchmark Layers

The project uses three benchmark layers:

| Layer | Tool | Purpose |
|---|---|---|
| Correctness | `allocator_validate`, `ctest` | Catch allocator API and memory corruption bugs before timing anything. |
| Built-in benchmarks | `bench_runner` | Fast local comparison with configurable workloads and JSON output. |
| External benchmarks | `scripts/external_bench.sh` | Stronger LD_PRELOAD tests using `mimalloc-bench`, Redis, glibc benchtests, and real programs when available. |

Recommended order:

```bash
ctest --test-dir build --output-on-failure
./build/allocator_validate --strategy hybrid
./build/allocator_validate --strategy ptmalloc
./build/bench_runner --strategy hybrid --profile smoke
scripts/external_bench.sh run-mimalloc-bench rptest
```

## 2. Build

```bash
cmake -B build .
cmake --build build -j$(nproc)
```

The main benchmark binary is:

```bash
./build/bench_runner --help
```

The LD_PRELOAD library is:

```text
build/libmy_ptmalloc.so
```

## 3. Built-in Strategy Selection

```bash
./build/bench_runner --strategy hybrid
./build/bench_runner --strategy ptmalloc
./build/bench_runner --strategy tcmalloc_like
./build/bench_runner --strategy jemalloc_like
./build/bench_runner --strategy mimalloc_like
./build/bench_runner --strategy adaptive
./build/bench_runner --strategy libc
./build/bench_runner --strategy plugin:./build/libcounting_malloc_strategy.so
```

| Strategy | Meaning |
|---|---|
| `hybrid` | Slab frontend plus ptmalloc-style fallback |
| `ptmalloc` | Chunk/bin/arena backend only |
| `tcmalloc_like` | Teaching thread-cache/central-list/span allocator |
| `jemalloc_like` | Teaching arena/run/tcache allocator |
| `mimalloc_like` | Teaching per-thread heap/page/remote-free allocator |
| `adaptive` | Independent two-layer adaptive backend |
| `libc` / `glibc` | System allocator baseline |
| `plugin:path.so` | External allocator strategy |

## 4. Built-in Profiles

```bash
./build/bench_runner --strategy hybrid --profile smoke
./build/bench_runner --strategy hybrid --profile micro
./build/bench_runner --strategy hybrid --profile stress
./build/bench_runner --strategy hybrid --profile all --json
```

| Profile | Included workloads | Use case |
|---|---|---|
| `smoke` | Small quick tests | Run after each allocator edit |
| `micro` | Same-size, batch, random | Compare hot paths |
| `stress` | Random, fragmentation, cross-thread free | Catch fragmentation and ownership weaknesses |
| `all` | All built-in workloads | Before documenting performance |

## 5. Built-in Workloads

| Workload | What it stresses |
|---|---|
| `same_size` | Fast path for one fixed allocation size |
| `same_size_64` | 64-byte hot path |
| `same_size_256` | 256-byte hot path |
| `batch` | Batch refill/drain and repeated allocation/free phases |
| `random` | Mixed allocation/free/realloc behavior |
| `fragmentation` | Realloc, split/coalesce policy, RSS pressure |
| `cross_thread_free` | Producer/consumer ownership behavior |
| `latency_sample` | Rough latency smoke check |

Examples:

```bash
./build/bench_runner --strategy hybrid --bench same_size --size 64 --iters 1000000
./build/bench_runner --strategy hybrid --bench random --random-iters 500000 --slots 8192 --min-size 16 --max-size 16384
./build/bench_runner --strategy hybrid --bench fragmentation --iters 100000 --slots 4096
./build/bench_runner --strategy hybrid --bench cross_thread_free --threads 8 --batch 10000
```

## 6. Parameters

| Option | Meaning |
|---|---|
| `--iters N` | Iteration count for fixed-size and fragmentation tests |
| `--random-iters N` | Iteration count for random mixed tests |
| `--size N` | Allocation size for same-size tests |
| `--min-size N` / `--max-size N` | Size distribution range |
| `--slots N` | Number of live pointer slots |
| `--batch N` | Batch size |
| `--rounds N` | Number of batch rounds |
| `--threads N` | Number of worker threads |
| `--repeats N` | Repeat each benchmark and output aggregate statistics |
| `--seed N` | Deterministic random seed |
| `--json` | Emit JSON lines |

Adaptive-specific runtime parameters can be set through the environment when `--strategy adaptive` or `MY_MALLOC_MODE=adaptive` is used:

| Env var | Meaning |
|---|---|
| `MY_MALLOC_ADAPTIVE_MODE` | `balanced`, `throughput_cache`, `deterministic_latency`, `compact_rss`, `fragmentation_stable`, `cross_thread`, `large_object`, `hardened_debug`, or `auto`. |
| `MY_MALLOC_ADAPTIVE_MODE_SELECTOR` | `rule`, `fixed`, or `manual`. |
| `MY_MALLOC_ADAPTIVE_MODE_WINDOW` | Rule-selector observation window. |
| `MY_MALLOC_ADAPTIVE_MODE_COOLDOWN` | Cooldown in windows after a mode switch. |
| `MY_MALLOC_ADAPTIVE_DEBUG_MODE` | Force `hardened_debug` when set to `1`. |
| `MY_MALLOC_ADAPTIVE_SMALL_PAGE_SIZE` | Initial small-object page size. |
| `MY_MALLOC_ADAPTIVE_MEDIUM_SPAN_SIZE` | Initial medium-object span size. |
| `MY_MALLOC_ADAPTIVE_EMPTY_CACHE_LIMIT` | Empty page/span keep count before release. |

Examples:

```bash
MY_MALLOC_ADAPTIVE_MODE=throughput_cache \
  ./build/bench_runner --strategy adaptive --bench throughput_server --json

MY_MALLOC_ADAPTIVE_MODE=compact_rss \
  ./build/bench_runner --strategy adaptive --bench memory_constrained --json

MY_MALLOC_ADAPTIVE_MODE=auto MY_MALLOC_ADAPTIVE_MODE_WINDOW=64 \
  ./build/bench_runner --strategy adaptive --bench producer_consumer --json
```

JSON example:

```bash
./build/bench_runner --strategy adaptive --bench same_size_64 --json
```

```json
{"strategy":"adaptive","benchmark":"same_size_64","ops_per_sec":24000000,"ms":41.000,"peak_rss_kb":31104,"adaptive":{"current_mode":"balanced","active_mode":"balanced","previous_mode":"balanced","mode_switches":0,"retired_mode_count":0,"mapped_bytes":65536,"live_bytes":0,"mapped_live_ratio":0.000,"remote_free_ratio":0.000,"size_entropy":0.000,"large_bytes_ratio":0.000,"fragmentation_estimate":0.000,"slow_path_ratio":0.000,"double_free_count":0,"invalid_free_count":0,"storage_allocs":[1,0,0],"storage_frees":[1,0,0],"pool_hits":[0,0,0],"pool_misses":[1,0,0]}}
```

Adaptive JSON is centered on the two-layer architecture: mode fields describe the policy layer, storage/cache/release fields describe the shared memory layer, and feature ratios describe selector inputs. Important adaptive fields are `current_mode`, `mode_switches`, per-mode alloc/free/live/mapped arrays, `storage_allocs`, `storage_frees`, `pool_hits`, `pool_misses`, `release_unmapped_bytes`, `mapped_live_ratio`, `remote_free_ratio`, `size_entropy`, `large_bytes_ratio`, `fragmentation_estimate`, and `slow_path_ratio`.

When `--repeats N` is greater than 1, JSON output reports aggregate `mean`, `median`, `p95`, `stddev`, `min`, and `max` operation rates instead of relying on a single sample.

## 7. External Benchmark Script

The helper script is:

```bash
scripts/external_bench.sh --help
```

It uses:

```text
external/          downloaded external repositories
results/external/  timestamped raw logs
```

These directories are ignored by Git except placeholder files.

## 8. mimalloc-bench

`mimalloc-bench` is a practical allocator benchmark suite used by the mimalloc project. It includes classic allocator tests such as Larson, alloc-test, cache-scratch, rptest, cfrac, espresso, and others.

Setup and build:

```bash
scripts/external_bench.sh setup-mimalloc-bench
scripts/external_bench.sh build-mimalloc-bench bench
```

Run selected tests:

```bash
scripts/external_bench.sh run-mimalloc-bench rptest larson alloc-test glibc-thread
```

For each selected test, the script runs:

```text
glibc system malloc
my_malloc hybrid mode through LD_PRELOAD
my_malloc ptmalloc mode through LD_PRELOAD
```

To include the teaching allocator-family modes, set `MIMALLOC_ALLOCATORS`:

```bash
MIMALLOC_ALLOCATORS="glibc my-hybrid my-ptmalloc my-tcmalloc-like my-jemalloc-like my-mimalloc-like my-adaptive" \
  scripts/external_bench.sh run-mimalloc-bench rptest
```

Important environment note: if the official `mimalloc-bench` build is blocked by missing tools such as `unzip`, the script falls back to a local CMake build of core benchmarks. In that fallback, shbench binaries are stubs and must not be reported.

## 9. Redis

Redis is a useful server-style workload with many small allocations and a client/server benchmark shape.

Build Redis through `mimalloc-bench`:

```bash
scripts/external_bench.sh build-mimalloc-bench redis
```

Run the Redis benchmark:

```bash
scripts/external_bench.sh run-redis
```

The script starts Redis three times:

```text
glibc server
hybrid server through LD_PRELOAD
ptmalloc server through LD_PRELOAD
```

To include the teaching modes:

```bash
REDIS_ALLOCATORS="glibc hybrid ptmalloc tcmalloc_like jemalloc_like mimalloc_like adaptive" \
  scripts/external_bench.sh run-redis
```

It then runs:

```bash
redis-benchmark -r 1000000 -n 100000 -q -P 16 lpush a 1 2 3 4 5 lrange a 1 5
```

Codex sandbox note: Redis needs local TCP sockets. In a restricted sandbox it may fail with `Can't create socket: Operation not permitted`. Run the script outside the sandbox or with approved network permissions.

## 10. glibc Benchtests

glibc has allocator benchtests, but this project does not vendor glibc.

Use an existing glibc source/build tree:

```bash
GLIBC_SRC=/path/to/glibc scripts/external_bench.sh run-glibc-benchtests "$GLIBC_SRC"
```

Specific tests:

```bash
GLIBC_SRC=/path/to/glibc scripts/external_bench.sh run-glibc-benchtests "$GLIBC_SRC" malloc-thread malloc-simple malloc-tcache
```

If the named tests are not present in that glibc tree, the script reports and skips them.

## 11. Real Application Smoke Tests

```bash
scripts/external_bench.sh run-real-apps
```

The script runs available tools and skips missing ones:

| Tool | Workload |
|---|---|
| `sqlite3` | In-memory insert/select workload |
| `clang++` | Syntax-check project source |
| `lua` | Large table of strings; Redis-vendored Lua is used if system Lua is missing |
| `z3` | Tiny SMT input |
| `redis-server` | Detection only; use `run-redis` for the actual server benchmark |

## 12. Reporting Rules

A benchmark result should include:

- commit hash;
- compiler and flags;
- CPU and OS;
- exact command;
- allocator strategy;
- workload parameters;
- elapsed time;
- throughput;
- peak RSS;
- number of repetitions;
- skipped tests and why they were skipped.

Do not compare:

- one allocator's best run against another allocator's average run;
- different workload parameters;
- fallback/stub external benchmarks as if they were official suite results;
- sandbox-failed Redis runs as allocator failures.

## 13. Current Result Summary

Latest selected external run on May 5, 2026:

| Workload | glibc | hybrid | ptmalloc mode |
|---|---:|---:|---:|
| `rptest` | 782,679 memory ops/CPU sec | 556,053 | 465,142 |
| `larson` | 33,747,387 ops/sec | 18,017,391 | 71,349 |
| `alloc-test` | 1.2B ops in 23,506 ms | 36,845 ms | 40,488 ms |
| `glibc-thread` | 158,122,468 iterations | 20,510,225 | 56,143,923 |
| Redis pipelined command | 184,162 req/sec | 139,665 | 74,516 |

Interpretation:

- The allocator now passes the selected external LD_PRELOAD workloads.
- Hybrid mode is useful on Redis relative to ptmalloc mode, but still trails glibc.
- RSS is still high on mixed/fragmentation-style workloads.
- Larson exposes severe ptmalloc-mode scalability weakness.
- The next meaningful performance work is empty slab/span reclamation, remote-free ownership, and better size-class/batch tuning.

Detailed raw-result notes are in [external_benchmark_results.md](external_benchmark_results.md).

## 14. Adaptive Mode Workloads

Use the mode-specific workloads to compare adaptive mode behavior:

```bash
cmake --build build -j2
ctest --test-dir build --output-on-failure

MY_MALLOC_ADAPTIVE_MODE=throughput_cache ./build/bench_runner --strategy adaptive --bench throughput_server --json
MY_MALLOC_ADAPTIVE_MODE=deterministic_latency ./build/bench_runner --strategy adaptive --bench realtime_latency --json
MY_MALLOC_ADAPTIVE_MODE=compact_rss ./build/bench_runner --strategy adaptive --bench memory_constrained --json
MY_MALLOC_ADAPTIVE_MODE=fragmentation_stable ./build/bench_runner --strategy adaptive --bench fragmentation --json
MY_MALLOC_ADAPTIVE_MODE=cross_thread ./build/bench_runner --strategy adaptive --bench producer_consumer --json
MY_MALLOC_ADAPTIVE_MODE=large_object ./build/bench_runner --strategy adaptive --bench large_streaming --json
MY_MALLOC_ADAPTIVE_MODE=hardened_debug ./build/bench_runner --strategy adaptive --bench debug_safety --json
```

JSON output includes mode telemetry, shared-memory storage telemetry, and selector features. Current measurements should be interpreted by mode (`current_mode`, per-mode counts), memory substrate behavior (`storage_allocs`, `pool_hits`, `pool_misses`, `release_unmapped_bytes`), and feature ratios (`remote_free_ratio`, `large_bytes_ratio`, `fragmentation_estimate`, `slow_path_ratio`, `mapped_live_ratio`). The intended comparisons are workload-specific: throughput mode should favor ops/sec on local short-lived allocations, compact mode should favor retained/mapped memory after phase changes, fragmentation-stable mode should reduce mapped/live and internal waste on mixed medium sizes, cross-thread mode should expose remote-free telemetry and owner reuse, large-object mode should isolate direct mappings, and hardened mode should report safety diagnostics.

External `mimalloc-bench glibc-simple` was also run through LD_PRELOAD for all modes:

| Allocator | Elapsed | Peak RSS KB | Status |
|---|---:|---:|---|
| glibc | 0:07.28 | 1,792 | pass |
| hybrid | 0:13.18 | 3,456 | pass |
| ptmalloc | 0:29.40 | 1,283,072 | pass |
| tcmalloc-like | 0:07.21 | 3,328 | pass |
| jemalloc-like | 0:08.79 | 3,584 | pass |
| mimalloc-like | 0:09.23 | 3,456 | pass |
| adaptive | 0:10.83 | 3,456 | pass |
