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
| `adaptive` | Configurable decision policy over all concrete implementations |
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
| `--seed N` | Deterministic random seed |
| `--json` | Emit JSON lines |

JSON example:

```bash
./build/bench_runner --strategy hybrid --profile micro --json
```

```json
{"strategy":"hybrid","benchmark":"same_size_64","ops_per_sec":44943821,"ms":0.222,"peak_rss_kb":15872}
```

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

## 14. Current Multi-mode Results

After adding the teaching allocator-family modes and adaptive policies, the following validation checks were run locally:

```bash
./build/allocator_validate --strategy tcmalloc_like
./build/allocator_validate --strategy jemalloc_like
./build/allocator_validate --strategy mimalloc_like
./build/allocator_validate --strategy adaptive
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=tcmalloc_like ./build/test_basic
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=jemalloc_like ./build/test_basic
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=mimalloc_like ./build/test_basic
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=adaptive ./build/test_basic
MY_MALLOC_ADAPTIVE_POLICY=bandit ./build/bench_runner --strategy adaptive --profile smoke --json
MY_MALLOC_ADAPTIVE_POLICY=fixed:mimalloc_like ./build/bench_runner --strategy adaptive --profile smoke --json
```

All passed.

Micro profile command:

```bash
for s in libc hybrid ptmalloc tcmalloc_like jemalloc_like mimalloc_like; do
  ./build/bench_runner --strategy "$s" --profile micro --json
done

for p in heuristic bandit round_robin fixed:hybrid fixed:ptmalloc fixed:tcmalloc_like fixed:jemalloc_like fixed:mimalloc_like; do
  MY_MALLOC_ADAPTIVE_POLICY="$p" ./build/bench_runner --strategy adaptive --profile micro --json
done
```

Micro profile results:

| Strategy / policy | `same_size_64` | `same_size_256` | `batch` | `random` | Peak RSS KB |
|---|---:|---:|---:|---:|---:|
| `libc` | 9,687,157 | 8,721,920 | 4,994,426 | 680,531 | 30,976 |
| `hybrid` | 19,066,447 | 23,997,094 | 7,294,658 | 410,129 | 31,104 |
| `ptmalloc` | 25,131,401 | 22,547,922 | 6,969,109 | 838,804 | 30,976 |
| `tcmalloc_like` | 26,750,315 | 28,639,500 | 10,593,669 | 868,243 | 30,976 |
| `jemalloc_like` | 24,157,087 | 26,551,248 | 8,583,202 | 597,775 | 30,976 |
| `mimalloc_like` | 27,092,665 | 30,810,164 | 9,526,397 | 918,838 | 30,976 |
| `adaptive:heuristic` | 25,075,164 | 28,697,567 | 6,478,567 | 703,114 | 30,976 |
| `adaptive:bandit` | 28,455,897 | 28,386,798 | 9,412,547 | 624,368 | 30,976 |
| `adaptive:round_robin` | 5,994,098 | 12,598,238 | 6,321,115 | 675,147 | 30,976 |
| `adaptive:fixed:hybrid` | 27,161,917 | 27,255,359 | 9,312,878 | 913,864 | 30,976 |
| `adaptive:fixed:ptmalloc` | 30,911,002 | 22,033,402 | 9,144,963 | 783,634 | 30,976 |
| `adaptive:fixed:tcmalloc_like` | 10,988,521 | 14,569,214 | 8,802,991 | 860,718 | 30,976 |
| `adaptive:fixed:jemalloc_like` | 32,355,493 | 31,322,763 | 10,648,904 | 1,039,885 | 30,976 |
| `adaptive:fixed:mimalloc_like` | 38,295,117 | 39,165,333 | 12,100,290 | 1,115,162 | 30,976 |

Stress-focused samples:

| Strategy / policy | `cross_thread_free` ops/sec | `fragmentation` ops/sec |
|---|---:|---:|
| `libc` | 1,521,286 | 177,916 |
| `hybrid` | 1,815,925 | 253,852 |
| `ptmalloc` | 929,072 | 301,377 |
| `tcmalloc_like` | 883,058 | 326,952 |
| `jemalloc_like` | 1,709,303 | 336,961 |
| `mimalloc_like` | 2,075,647 | 203,278 |
| `adaptive:heuristic` | 1,536,604 | 267,972 |
| `adaptive:bandit` | 1,623,024 | not run |
| `adaptive:fixed:tcmalloc_like` | 1,151,861 | not run |
| `adaptive:fixed:jemalloc_like` | 1,347,910 | not run |
| `adaptive:fixed:mimalloc_like` | 1,113,186 | not run |

Interpretation:

- Against `libc`, the teaching allocators are faster on this micro matrix, but that does not imply production superiority.
- `mimalloc_like` is strongest on this cross-thread sample, which matches its owner/remote-free design goal.
- The current default adaptive heuristic is not best-in-class on these tests. It is a configurable decision module baseline, not an optimized learned policy.
- `fixed:mimalloc_like` wins this micro matrix, while `jemalloc_like` and `tcmalloc_like` are stronger on the fragmentation sample. This confirms why adaptive needs workload signals and feedback rather than a single static size table.
- `round_robin` performs poorly on same-size hot paths because it deliberately mixes incompatible implementation choices. It is useful as a stress policy, not as an optimization policy.

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
