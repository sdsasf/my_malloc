# External Benchmark Results

This document records the latest external benchmark status from the local development environment on May 5, 2026. Raw logs are written under `results/external/` and are ignored by Git.

For commands and methodology, read [benchmarking.md](benchmarking.md).

## 1. Environment

| Item | Value |
|---|---|
| OS | Ubuntu 22.04.5 LTS |
| Worker count used by scripts | 12 |
| Allocator library | `build/libmy_ptmalloc.so` |
| External suite | `external/mimalloc-bench` |
| Redis source | `external/mimalloc-bench/extern/redis-6.2.7` |
| Build mode | Core `mimalloc-bench` fallback plus Redis build |

Environment blockers:

| Blocker | Effect |
|---|---|
| `unzip` not installed | Official `mimalloc-bench` shbench setup could not complete. |
| `sudo -n true` requires password | System packages could not be installed from this session. |
| MicroQuill shbench download returned wrong content/checksum | shbench results are not valid and are not reported. |
| No local glibc source tree | glibc benchtests were not run. |
| `sqlite3`, `clang++`, `z3` missing | These real-app smoke workloads were skipped. |
| Sandbox blocks local TCP sockets | Redis must be run outside the restricted sandbox or with approved network permissions. |

## 2. Crash Fixes Before Latest Run

Earlier LD_PRELOAD runs exposed `MY_MALLOC_MODE=ptmalloc` crashes. The latest run was performed after these fixes:

| Area | Problem | Fix |
|---|---|---|
| Allocator initialization | `AllocPipeline` used `std::vector`; vector construction could call `operator new`, re-enter `malloc`, and call the pipeline before global initialization completed. | Replace strategy storage with fixed `std::array<AllocStrategy*, 7>`. |
| Hook bootstrap | Some early allocations were served by libc malloc during hook initialization, then later freed through this allocator. | Track real-bootstrap pointers and route their `free`/`realloc` back to libc. |
| `memalign` | Alignment requests `<= MALLOC_ALIGNMENT` took the aligned split path even though normal malloc already satisfies the alignment. Stress tests could corrupt bin links. | Route those requests directly to `my_malloc`. |

Validation after fixes:

```bash
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/allocator_validate --strategy hybrid
./build/allocator_validate --strategy ptmalloc
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=ptmalloc ./build/test_basic
```

All of the above passed.

## 3. Commands Run

```bash
scripts/external_bench.sh setup-mimalloc-bench
scripts/external_bench.sh build-mimalloc-bench bench
scripts/external_bench.sh build-mimalloc-bench redis
scripts/external_bench.sh run-mimalloc-bench rptest larson alloc-test glibc-thread
scripts/external_bench.sh run-redis
scripts/external_bench.sh run-real-apps
```

`run-redis` was executed outside the restricted sandbox because Redis needs local TCP sockets.

## 4. mimalloc-bench Results

Lower elapsed time is better. Higher throughput/iteration counts are better. RSS is peak resident set size from `/usr/bin/time`.

| Benchmark | Allocator | Main metric | Elapsed | Peak RSS KB | Status |
|---|---|---:|---:|---:|---|
| `rptest` | glibc | 782,679 memory ops/CPU sec | 0:17.22 | 53,620 | pass |
| `rptest` | hybrid | 556,053 memory ops/CPU sec | 0:16.02 | 113,532 | pass |
| `rptest` | ptmalloc | 465,142 memory ops/CPU sec | 0:17.22 | 112,296 | pass |
| `larson` | glibc | 33,747,387 ops/sec | 0:07.09 | 78,088 | pass |
| `larson` | hybrid | 18,017,391 ops/sec | 0:07.14 | 290,432 | pass |
| `larson` | ptmalloc | 71,349 ops/sec | 0:08.36 | 46,720 | pass |
| `alloc-test` | glibc | 1.2B ops in 23,506 ms | 0:23.51 | 15,616 | pass |
| `alloc-test` | hybrid | 1.2B ops in 36,845 ms | 0:36.85 | 27,776 | pass |
| `alloc-test` | ptmalloc | 1.2B ops in 40,488 ms | 0:40.52 | 132,864 | pass |
| `glibc-thread` | glibc | 158,122,468 iterations | 0:02.01 | 3,200 | pass |
| `glibc-thread` | hybrid | 20,510,225 iterations | 0:02.03 | 51,072 | pass |
| `glibc-thread` | ptmalloc | 56,143,923 iterations | 0:02.01 | 13,056 | pass |

Note: one older `ptmalloc/rptest` log still contains a SIGSEGV from before the final successful rerun. An immediate GDB run and a later full rerun completed normally.

## 5. Redis Result

Redis command:

```bash
redis-benchmark -r 1000000 -n 100000 -q -P 16 lpush a 1 2 3 4 5 lrange a 1 5
```

| Allocator | Requests/sec | p50 | Elapsed | Peak RSS KB | Status |
|---|---:|---:|---:|---:|---|
| glibc | 184,162.06 | 3.935 ms | 0:00.55 | 3,840 | pass |
| hybrid | 139,664.80 | 5.383 ms | 0:00.72 | 3,840 | pass |
| ptmalloc | 74,515.65 | 9.863 ms | 0:01.35 | 3,840 | pass |

Interpretation:

- Hybrid mode is much closer to glibc than ptmalloc mode on this Redis workload.
- The slab frontend helps the server-style small-object pattern.
- Hybrid still trails glibc, so the current slab implementation is not yet production-level.

## 6. Real Application Smoke Tests

`run-real-apps` result:

| Tool | Result |
|---|---|
| SQLite | skipped, `sqlite3` not installed |
| clang++ | skipped, `clang++` not installed |
| Lua | passed using Redis-vendored Lua |
| Z3 | skipped, `z3` not installed |
| Redis | Redis is built; use `run-redis` for the actual benchmark |

Lua workload output:

```text
200000  199999:39999600001
```

## 7. What The Results Mean

The allocator is now correct enough to survive the selected external LD_PRELOAD tests, but its performance gap is still clear.

Observed gaps:

| Gap | Evidence | Likely cause |
|---|---|---|
| High RSS on `rptest` | hybrid/ptmalloc RSS is about 2x glibc | No empty slab/span release and weaker reuse policy |
| Poor ptmalloc Larson throughput | ptmalloc mode far behind glibc/hybrid | Arena/bin contention and slow fallback path |
| Hybrid slower than glibc on Redis | 139k req/sec vs 184k req/sec | Slab frontend helps, but batch/size-class/cache policy is not tuned |
| Hybrid weak on `glibc-thread` | far fewer iterations than glibc | Thread-local/central handoff overhead and simplified cache design |

## 8. Next Benchmark Work

1. Add repeated-run summary generation from `results/external/*.log`.
2. Add variance reporting: min, median, p95, standard deviation.
3. Add live bytes, mapped bytes, free bytes, and slab/arena hit-rate counters.
4. Install or provide `unzip`, glibc source, SQLite, clang, Z3, jemalloc, tcmalloc, and mimalloc for broader comparison.
5. Re-run the same matrix after implementing empty slab/span release and remote-free queues.
