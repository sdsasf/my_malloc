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
| `adaptive` | Independent adaptive backend with internal strategy policy |
| `adaptive_demo` / `demo_all` | Legacy demo policy over teaching implementations |
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
| `MY_MALLOC_ADAPTIVE_POLICY` | Architecture policy: `heuristic`, `fixed:small`, `fixed:medium`, `fixed:large`, `round_robin`, `epsilon_greedy`, `ucb1`, `thompson_sampling`. |
| `MY_MALLOC_ADAPTIVE_PARAM_POLICY` | Parameter policy: `static`, `heuristic`, `coordinate_bandit`, `bayesian_offline`. |
| `MY_MALLOC_ADAPTIVE_ARCH_WINDOW` | Allocation window for telemetry-driven architecture switching. |
| `MY_MALLOC_ADAPTIVE_PARAM_WINDOW` | Allocation window for parameter tuning. |
| `MY_MALLOC_ADAPTIVE_SMALL_PAGE_SIZE` | Initial small-object page size. |
| `MY_MALLOC_ADAPTIVE_MEDIUM_SPAN_SIZE` | Initial medium-object span size. |
| `MY_MALLOC_ADAPTIVE_LOCAL_BATCH` | Batch-size hint for future local-cache work. |
| `MY_MALLOC_ADAPTIVE_EMPTY_CACHE_LIMIT` | Empty page/span keep count before release. |
| `MY_MALLOC_ADAPTIVE_COOLDOWN_WINDOWS` | Cooldown after architecture/profile switches. |
| `MY_MALLOC_ADAPTIVE_PROFILE` | Initial profile: `balanced`, `low_latency`, `low_rss`, `large_heavy`, `cross_thread`. |

Examples:

```bash
MY_MALLOC_ADAPTIVE_POLICY=ucb1 MY_MALLOC_ADAPTIVE_PARAM_POLICY=heuristic \
  ./build/bench_runner --strategy adaptive --profile micro --json

MY_MALLOC_ADAPTIVE_POLICY=ucb1 MY_MALLOC_ADAPTIVE_PARAM_POLICY=static \
  MY_MALLOC_ADAPTIVE_SMALL_PAGE_SIZE=32768 \
  ./build/bench_runner --strategy adaptive --bench same_size --size 64

./build/bench_runner --strategy adaptive --bench phase_changing --json
./build/bench_runner --strategy adaptive --bench same_size_64 --repeats 5 --json
```

JSON example:

```bash
./build/bench_runner --strategy hybrid --profile micro --json
```

```json
{"strategy":"adaptive","benchmark":"same_size_64","ops_per_sec":24430358,"ms":40.933,"peak_rss_kb":31104,"adaptive":{"architecture_switches":0,"parameter_decisions":0,"config_version":1,"profile":"balanced","mapped_bytes":0,"live_bytes":0,"mapped_live_ratio":0.000,"empty_pages":0,"empty_spans":0,"released_pages":0,"released_spans":0,"release_unmapped_bytes":0,"strategy_allocs":[1000,0,0],"strategy_frees":[1000,0,0],"pool_hits":[999,0,0],"pool_misses":[1,0,0]}}
```

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

## 14. Current Multi-mode Results

The latest focused local run was performed on May 6, 2026 after adding windowed architecture switching and the separate parameter tuning layer.

```bash
cmake --build build -j2
ctest --test-dir build --output-on-failure

./build/bench_runner --strategy libc --profile micro --json
./build/bench_runner --strategy adaptive --profile micro --json
MY_MALLOC_ADAPTIVE_POLICY=ucb1 MY_MALLOC_ADAPTIVE_PARAM_POLICY=static ./build/bench_runner --strategy adaptive --profile micro --json
MY_MALLOC_ADAPTIVE_POLICY=ucb1 MY_MALLOC_ADAPTIVE_PARAM_POLICY=heuristic ./build/bench_runner --strategy adaptive --profile micro --json
MY_MALLOC_ADAPTIVE_POLICY=ucb1 MY_MALLOC_ADAPTIVE_PARAM_POLICY=coordinate_bandit ./build/bench_runner --strategy adaptive --profile micro --json
./build/bench_runner --strategy libc --profile stress --json
MY_MALLOC_ADAPTIVE_POLICY=ucb1 MY_MALLOC_ADAPTIVE_PARAM_POLICY=heuristic ./build/bench_runner --strategy adaptive --profile stress --json
```

### 14.1 Latest Adaptive Two-layer Sample

| Strategy / config | `same_size_64` | `same_size_256` | `batch` | `random` | Peak RSS KB |
|---|---:|---:|---:|---:|---:|
| `libc` | 12,574,450 | 9,694,595 | 7,019,472 | 996,505 | 30,976 |
| `adaptive` default | 24,430,358 | 24,197,039 | 9,799,662 | 814,750 | 31,104 |
| `adaptive`, `ucb1+static` | 27,902,697 | 28,317,132 | 10,642,325 | 840,813 | 30,848 |
| `adaptive`, `ucb1+heuristic` | 24,653,533 | 25,055,846 | 9,568,574 | 1,047,587 | 31,104 |
| `adaptive`, `ucb1+coordinate_bandit` | 20,157,150 | 20,690,505 | 9,333,185 | 927,191 | 30,976 |

Stress sample:

| Strategy / config | `random` | `fragmentation` | `cross_thread_free` | Peak RSS KB |
|---|---:|---:|---:|---:|
| `libc` | 1,304,079 | 1,392,338 | 881,577 | 58,880 |
| `adaptive`, `ucb1+heuristic` | 1,302,661 | 1,613,490 | 820,079 | 59,008 |

Interpretation:

- Windowed adaptive routing keeps hot-path micro throughput competitive with `libc` in this local run.
- `ucb1+static` is strongest among the tested configurations for steady same-size and batch micro workloads.
- `ucb1+heuristic` is strongest among the tested adaptive configurations for random micro and fragmentation stress.
- Cross-thread stress still trails `libc`; owner-thread queues and per-thread/per-CPU adaptive caches remain important next steps.

The broader multi-mode run below was performed on May 5, 2026 after adding the independent adaptive allocator, adaptive small/medium page-span pools, and telemetry-driven adaptive policies. It is kept as a historical comparison across all teaching allocators.

```bash
cmake --build build -j2
ctest --test-dir build --output-on-failure

for s in libc hybrid ptmalloc tcmalloc_like jemalloc_like mimalloc_like adaptive; do
  ./build/bench_runner --strategy "$s" --profile micro --json
  ./build/bench_runner --strategy "$s" --profile stress --json
done
```

The run compares the teaching allocator implementations, the self-developed `adaptive` allocator, and `libc` as a baseline. Values are operations per second; higher is better. Peak RSS is the maximum KB observed in that profile's subtests.

### 14.2 Allocator Micro Profile

| Strategy | `same_size_64` | `same_size_256` | `batch` | `random` | Peak RSS KB |
|---|---:|---:|---:|---:|---:|
| `libc` | 12,822,325 | 13,022,034 | 7,127,544 | 1,046,455 | 31,104 |
| `hybrid` | 27,823,527 | 27,500,649 | 10,936,918 | 1,102,044 | 31,104 |
| `ptmalloc` | 26,197,541 | 26,693,840 | 11,054,527 | 1,157,274 | 30,976 |
| `tcmalloc_like` | 26,748,702 | 10,528,622 | 3,796,048 | 759,696 | 30,848 |
| `jemalloc_like` | 18,887,703 | 19,252,725 | 8,979,967 | 816,400 | 31,104 |
| `mimalloc_like` | 23,087,757 | 22,974,563 | 9,218,316 | 947,105 | 30,976 |
| `adaptive` | 23,284,677 | 12,417,979 | 3,073,215 | 827,614 | 30,976 |

### 14.3 Allocator Stress Profile

| Strategy | `random` | `fragmentation` | `cross_thread_free` | Peak RSS KB |
|---|---:|---:|---:|---:|
| `libc` | 1,027,949 | 1,438,969 | 842,192 | 59,008 |
| `hybrid` | 1,371,708 | 1,306,031 | 835,924 | 58,880 |
| `ptmalloc` | 1,549,313 | 1,559,973 | 696,488 | 59,008 |
| `tcmalloc_like` | 1,161,121 | 1,518,785 | 1,094,520 | 58,752 |
| `jemalloc_like` | 1,622,627 | 1,813,802 | 976,109 | 58,880 |
| `mimalloc_like` | 1,599,208 | 1,517,392 | 732,602 | 58,860 |
| `adaptive` | 1,296,166 | 1,380,093 | 910,505 | 58,752 |

### 14.4 Adaptive Policy Micro Profile

Command:

```bash
for p in heuristic epsilon_greedy ucb1 thompson_sampling round_robin fixed:small fixed:medium fixed:large; do
  MY_MALLOC_ADAPTIVE_POLICY="$p" ./build/bench_runner --strategy adaptive --profile micro --json
done
```

| Adaptive policy | `same_size_64` | `same_size_256` | `batch` | `random` | Peak RSS KB |
|---|---:|---:|---:|---:|---:|
| `heuristic` | 8,185,376 | 21,523,521 | 8,230,347 | 844,231 | 30,976 |
| `epsilon_greedy` | 19,812,613 | 24,467,825 | 8,217,441 | 923,744 | 30,976 |
| `ucb1` | 22,236,701 | 24,847,005 | 9,492,521 | 922,979 | 30,976 |
| `thompson_sampling` | 18,707,184 | 15,425,862 | 2,483,404 | 796,735 | 30,976 |
| `round_robin` | 22,045,714 | 23,955,222 | 10,005,564 | 1,071,706 | 30,976 |
| `fixed:small` | 22,356,535 | 24,069,442 | 8,743,437 | 864,238 | 30,976 |
| `fixed:medium` | 23,401,058 | 23,942,701 | 9,224,166 | 1,074,517 | 30,976 |
| `fixed:large` | 26,608,120 | 25,306,594 | 3,898,270 | 820,242 | 31,104 |

### 14.5 Adaptive Policy Stress Profile

| Adaptive policy | `random` | `fragmentation` | `cross_thread_free` | Peak RSS KB |
|---|---:|---:|---:|---:|
| `heuristic` | 889,083 | 1,694,512 | 753,009 | 58,880 |
| `ucb1` | 1,571,848 | 1,688,021 | 691,817 | 58,752 |
| `thompson_sampling` | 731,785 | 1,595,027 | 836,038 | 58,860 |
| `epsilon_greedy` | 1,528,203 | 1,488,548 | 941,306 | 58,880 |

Interpretation:

- `hybrid` and `ptmalloc` are strongest in this micro matrix on fixed-size and batch workloads.
- `jemalloc_like` leads the stress fragmentation sample, which matches its arena/run organization goal in this simplified benchmark.
- `tcmalloc_like` leads the stress cross-thread sample in this run, while `mimalloc_like` is not yet showing its expected remote-free advantage. That points to tuning gaps in the simplified mimalloc-like page/remote-free model or in the benchmark shape.
- `adaptive` is competitive in some stress cases but still pays overhead from policy/telemetry, the page ownership filter, per-class locking, and conservative page/span release.
- Among adaptive policies, `ucb1` is a strong telemetry-driven baseline on micro batch/random and stress random/fragmentation in this run. `epsilon_greedy` leads adaptive stress cross-thread. `thompson_sampling` is currently less stable and should be treated as a baseline, not a tuned model.

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
