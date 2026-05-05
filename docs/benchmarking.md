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
./build/bench_runner --strategy libc
./build/bench_runner --strategy plugin:./build/libcounting_malloc_strategy.so
```

| Strategy | Meaning |
|---|---|
| `hybrid` | Slab frontend plus ptmalloc-style fallback |
| `ptmalloc` | Chunk/bin/arena backend only |
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
