# Adaptive Two-Layer Allocator

`MY_MALLOC_MODE=adaptive` selects an independent adaptive allocator backend. It is not a dispatcher over the teaching allocators. The architecture has one clear main line:

```text
Adaptive Allocator
  = Shared Memory Management Layer
  + Adaptive Mode Policy Layer
  + Runtime Telemetry and Mode Selector
```

The shared layer owns memory. Modes are policies that describe how to use that shared memory substrate. The selector observes workload features and soft-switches the active mode for future allocations only.

## Shared Memory Management Layer

Files:

- `include/my_ptmalloc/adaptive_services.h`
- `src/adaptive_services.cpp`
- shared types in `include/my_ptmalloc/adaptive_types.h`

Responsibilities:

- maintain `AdaptiveHeader`;
- maintain ownership page table and registry lookup;
- manage size-class pages for small objects;
- manage span-backed storage for medium objects;
- manage extent/direct mappings for large, aligned, and debug objects;
- manage central free lists and shared pools;
- execute reclaim decisions: cache, purge, unmap, quarantine;
- record object owner thread, requested size, usable size, mapped size, and allocation-time `mode_id`;
- emit raw telemetry events.

This layer has no mode-selection logic and does not read the active mode. It only executes an `AllocationRequest` or `ReleaseDecision` produced by the mode layer.

## Adaptive Mode Policy Layer

Files:

- `include/my_ptmalloc/adaptive_mode.h`
- `src/adaptive_modes.cpp`

Modes return:

- `AllocationPlan`: storage preference, cache/reuse/RSS/debug intent, batch hint, empty keep limit;
- `ReleaseDecision`: cache, return to central pool, purge, unmap, or quarantine, plus poison/redzone flags.

Modes do not directly mutate page/span/mmap internals.

| Mode | AllocationPlan | ReleaseDecision | Current status |
|---|---|---|---|
| `Balanced` | `Auto`, reuse enabled, default empty keep limit | return to central pool | Stable baseline |
| `ThroughputCache` | size-class/span preference, reuse, thread-cache semantic hook, larger batch, higher keep limit | cache | Real TLS cache is TODO |
| `DeterministicLatency` | `Auto`, reuse, moderate batch, avoids aggressive release | return to central pool | Stable reuse policy; p99 sampler TODO |
| `CompactRSS` | low-RSS, no thread cache, direct map for large objects, keep limit 0 | purge or unmap | Aggressive reclaim implemented |
| `FragmentationStable` | `Auto`, reuse, low keep limit, fragmentation telemetry hook | return to central pool | Size-class rebalance TODO |
| `CrossThreadMessage` | `Auto`, owner/remote-free semantic hook | return to central pool | Remote-free telemetry implemented; queue TODO |
| `LargeObjectStreaming` | direct map for objects >= 4 KiB, size-class for small | unmap direct mappings | Large object isolation implemented |
| `HardenedDebug` | direct map, no reuse, debug redzone/quarantine intent | quarantine with poison/check hooks | Header checks, counters, poison/quarantine mapping retention implemented; full redzone TODO |

## Runtime Telemetry and Selector

Files:

- `include/my_ptmalloc/adaptive_telemetry.h`
- `src/adaptive_telemetry.cpp`
- `include/my_ptmalloc/adaptive_selector.h`
- `src/adaptive_selector.cpp`
- `include/my_ptmalloc/adaptive_runtime.h`
- `src/adaptive_runtime.cpp`

Telemetry tracks:

- malloc/free/realloc/failure counts;
- per-mode alloc/free/live/mapped stats;
- per-storage alloc/free/requested/usable/cache hit/cache miss stats;
- live bytes and mapped bytes;
- same-thread and remote-free counts;
- invalid free, double free, and header corruption counts;
- mmap/munmap/slow-path counts;
- size histogram;
- empty/released page/span counts.

`WorkloadFeatures` includes:

- small/medium/large object ratios;
- large bytes ratio;
- size entropy;
- cache hit rate and reuse rate;
- remote-free ratio;
- mapped/live and retained ratios;
- internal fragmentation ratio;
- external fragmentation score;
- slow-path ratio;
- safety error rate.

Phase 1 computes these from cumulative counters as a coarse window approximation. The API is window-shaped so it can become a true sliding-window delta without changing mode policy code.

## Selector Rules

`MY_MALLOC_ADAPTIVE_MODE=auto` with `MY_MALLOC_ADAPTIVE_MODE_SELECTOR=rule` enables rule selection:

1. debug flag or high safety error rate -> `HardenedDebug`;
2. high mapped/live pressure -> `CompactRSS`;
3. high remote-free ratio -> `CrossThreadMessage`;
4. high large-bytes ratio -> `LargeObjectStreaming`;
5. high slow-path ratio -> `DeterministicLatency`;
6. high size entropy plus fragmentation -> `FragmentationStable`;
7. high cache hit/reuse rate with low RSS pressure -> `ThroughputCache`;
8. otherwise -> `Balanced`.

Runtime guards:

- `MY_MALLOC_ADAPTIVE_MODE_WINDOW` controls observation cadence;
- `MY_MALLOC_ADAPTIVE_MODE_COOLDOWN` prevents switch thrashing;
- expected-gain thresholds provide hysteresis;
- `MY_MALLOC_ADAPTIVE_DEBUG_MODE=1` forces `HardenedDebug`;
- `MY_MALLOC_ADAPTIVE_MODE_SELECTOR=fixed` keeps the configured mode;
- `manual` allows tests to call `adaptive_set_mode()`.

## Soft Switch Invariants

- `adaptive_malloc` reads the active mode.
- The mode returns an `AllocationPlan`.
- `MemoryServices` executes the plan and writes `AdaptiveHeader::mode_id`.
- `adaptive_free`, `adaptive_realloc`, and `adaptive_usable_size` route by allocation-time `mode_id`.
- Mode switches affect future allocations only.
- Live objects are not migrated.
- All modes share the same memory substrate.

## Public Configuration

| Env var | Values |
|---|---|
| `MY_MALLOC_ADAPTIVE_MODE` | `balanced`, `throughput_cache`, `deterministic_latency`, `compact_rss`, `fragmentation_stable`, `cross_thread`, `large_object`, `hardened_debug`, `auto` |
| `MY_MALLOC_ADAPTIVE_MODE_SELECTOR` | `rule`, `fixed`, `manual` |
| `MY_MALLOC_ADAPTIVE_MODE_WINDOW` | selector observation window |
| `MY_MALLOC_ADAPTIVE_MODE_COOLDOWN` | cooldown in windows |
| `MY_MALLOC_ADAPTIVE_DEBUG_MODE` | `0` / `1` |
| `MY_MALLOC_ADAPTIVE_SMALL_PAGE_SIZE` | initial size-class page size |
| `MY_MALLOC_ADAPTIVE_MEDIUM_SPAN_SIZE` | initial span size |
| `MY_MALLOC_ADAPTIVE_EMPTY_CACHE_LIMIT` | default retained empty page/span count |

## Benchmark JSON

`bench_runner --json` reports adaptive fields:

- `current_mode`, `active_mode`, `previous_mode`;
- `mode_switches`, `retired_mode_count`;
- `mode_alloc_count`, `mode_free_count`;
- `mode_live_bytes`, `mode_mapped_bytes`;
- `storage_allocs`, `storage_frees`, `pool_hits`, `pool_misses`;
- `remote_free_ratio`, `size_entropy`, `large_bytes_ratio`;
- `mapped_live_ratio`, `fragmentation_estimate`, `slow_path_ratio`;
- `double_free_count`, `invalid_free_count`.

## Current Limitations

Implemented:

- split shared memory services / mode policy / telemetry selector files;
- allocation-time `mode_id`;
- shared ownership table and registry;
- size-class pages, spans, direct mappings;
- per-mode and per-storage stats;
- remote-free telemetry;
- rule selector with window/cooldown/hysteresis;
- compact RSS purge/unmap behavior;
- large-object direct-map isolation;
- hardened debug poison and quarantine mapping retention.

Future work:

- real thread-local cache service;
- bounded remote-free queue and owner-aware reclaim;
- fragmentation-stable size-class table policy;
- sampled p95/p99 latency;
- full redzone/canary validation;
- true sliding-window feature deltas.

## Commands

```bash
./build/allocator_validate --strategy adaptive
MY_MALLOC_ADAPTIVE_MODE=compact_rss ./build/allocator_validate --strategy adaptive
MY_MALLOC_ADAPTIVE_MODE=auto MY_MALLOC_ADAPTIVE_MODE_WINDOW=64 ./build/bench_runner --strategy adaptive --bench cross_thread_free --json
MY_MALLOC_ADAPTIVE_MODE=large_object ./build/bench_runner --strategy adaptive --bench phase_changing --json
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=adaptive ./build/test_basic
```
