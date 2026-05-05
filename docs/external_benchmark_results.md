# External Benchmark Run Notes

This file records the external benchmark run performed on May 5, 2026 in the local development environment. Raw logs are written under `results/external/` and are intentionally ignored by Git.

## Environment

| Item | Value |
|---|---|
| OS | Ubuntu 22.04.5 LTS |
| CPU workers used by scripts | 12 |
| Project build | `build/libmy_ptmalloc.so` |
| External suite | `external/mimalloc-bench` cloned from `https://github.com/daanx/mimalloc-bench` |
| Build mode | Official `build-bench-env.sh bench` attempted, then local fallback CMake build used |

The official `mimalloc-bench` build was blocked by local environment issues:

- `unzip` is not installed;
- `sudo apt-get` could not run because this session has no interactive password input;
- the MicroQuill shbench zip download did not match the checksum expected by `mimalloc-bench`.

The integration script therefore used its fallback path: it built the core `mimalloc-bench` binaries through the suite's CMake project and generated shbench stubs only to let CMake configure. shbench results should not be reported from this run.

## Commands Run

```bash
scripts/external_bench.sh setup-mimalloc-bench
scripts/external_bench.sh build-mimalloc-bench bench
scripts/external_bench.sh run-mimalloc-bench larson alloc-test cscratch glibc-simple glibc-thread xmalloc-test malloc-large
scripts/external_bench.sh run-mimalloc-bench larson-sized cthrash mstress mleak rptest cfrac espresso barnes
scripts/external_bench.sh run-real-apps
```

glibc benchtests were not run because no local glibc source/build tree with `benchtests/` was found. `run-real-apps` found that `sqlite3`, `clang++`, `lua`, `z3`, and `redis-server` were not installed, so those real-application smoke workloads were skipped.

## mimalloc-bench Core Results

The table reports wall-clock elapsed time and peak RSS from `/usr/bin/time`. Lower elapsed time is better. Lower RSS is usually better, but it should be interpreted together with workload behavior.

| Benchmark | glibc elapsed | glibc RSS KB | hybrid elapsed | hybrid RSS KB | ptmalloc mode |
|---|---:|---:|---:|---:|---|
| `larson` | 0:07.13 | 78,388 | 0:07.16 | 341,888 | SIGSEGV |
| `larson-sized` | 0:07.12 | 78,572 | 0:07.16 | 336,768 | SIGSEGV |
| `alloc-test` | 0:21.56 | 15,360 | 0:25.79 | 33,664 | SIGSEGV |
| `cscratch` | 0:00.58 | 3,712 | 0:00.61 | 4,096 | SIGSEGV |
| `cthrash` | 0:00.64 | 3,712 | 0:00.66 | 3,968 | SIGSEGV |
| `glibc-simple` | 0:05.87 | 2,048 | 0:08.19 | 3,456 | SIGSEGV |
| `glibc-thread` | 0:02.00 | 3,456 | 0:02.03 | 47,616 | SIGSEGV |
| `xmalloc-test` | 0:05.04 | 73,404 | 0:05.01 | 9,544 | SIGSEGV |
| `malloc-large` | 0:05.31 | 534,192 | 0:31.53 | 410,032 | SIGSEGV |
| `mstress` | 0:03.19 | 315,052 | 0:04.19 | 938,288 | SIGSEGV |
| `mleak` | 0:06.07 | 2,048 | 0:06.52 | 203,136 | SIGSEGV |
| `rptest` | 0:16.02 | 49,592 | 0:16.03 | 130,440 | SIGSEGV |
| `cfrac` | 0:09.86 | 2,944 | 0:12.19 | 4,480 | SIGSEGV |
| `espresso` | 0:07.46 | 2,304 | 0:08.27 | 9,216 | SIGSEGV |
| `barnes` | 0:04.37 | 58,368 | 0:05.35 | 60,032 | SIGSEGV |

Additional workload-specific outputs:

| Benchmark | glibc output | hybrid output |
|---|---|---|
| `larson` | 27,689,364 ops/sec | 21,563,728 ops/sec |
| `larson-sized` | 28,457,840 ops/sec | 21,233,367 ops/sec |
| `alloc-test` | 1.2B operations in 21,557 ms | 1.2B operations in 25,783 ms |
| `xmalloc-test` | `rtime: 5.101`, `free/sec: 19.604 M` | `rtime: 25.815`, `free/sec: 3.874 M` |
| `rptest` | 669,230 memory ops/CPU second | 500,025 memory ops/CPU second |

## Interpretation

The hybrid allocator is functional under the external core benchmark set, but it is usually slower than glibc on these broader workloads and often uses more RSS. The important exceptions are workload-specific: `xmalloc-test` shows similar elapsed time and lower peak RSS in this run, but its own printed `rtime/free/sec` metric is worse for hybrid, so it needs deeper interpretation.

The `ptmalloc` runtime mode is not externally robust yet. It crashes quickly under every `mimalloc-bench` core workload tested through `LD_PRELOAD`. This should be treated as a correctness bug in the ptmalloc-only path before using that mode for external comparisons.

The main performance gaps exposed by this run match the current architecture limitations:

- no empty slab/span reclamation, causing high RSS on several workloads;
- no owner-thread remote-free lists, hurting cross-thread and producer/consumer patterns;
- simplified large allocation/page management, visible in `malloc-large`;
- ptmalloc-only mode has a crash bug under external LD_PRELOAD workloads;
- no tuned size-class table or span/page-map layer yet.

## Next Fix Targets

1. Reproduce and fix the ptmalloc-mode SIGSEGV under a smaller external test, starting with `glibc-simple` or `cscratch`.
2. Add empty slab/span accounting and return fully empty slabs to a central span cache or the OS.
3. Add owner-thread remote-free lists for slab objects.
4. Add structured parsing for `results/external/*.log` so benchmark reports can be generated automatically.
5. Re-run after installing `unzip` and full `mimalloc-bench` dependencies, then include `sh6bench`, `sh8bench`, Redis, RocksDB, Lua, Z3, and allocator comparisons against jemalloc/tcmalloc/mimalloc.
