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
- manage extent/direct mappings for true large, aligned, and debug objects;
- manage central free lists, thread-local magazines, remote-free queues, and shared pools;
- pack or spread allocations according to the mode's fragmentation policy;
- cache bounded large extents for streaming reuse;
- execute reclaim decisions: cache, purge, unmap, quarantine;
- record object owner thread, requested size, usable size, mapped size, and allocation-time `mode_id`;
- emit raw telemetry events.

This layer has no mode-selection logic and does not read the active mode. It only executes an `AllocationRequest` or `ReleaseDecision` produced by the mode layer.

## Adaptive Mode Policy Layer

Files:

- `include/my_ptmalloc/adaptive_mode.h`
- `src/adaptive_modes.cpp`

Modes return:

- `AllocationPlan`: storage preference, cache/reuse/RSS/debug intent, remote-drain policy, occupancy-packing policy, batch hint, tcache limits, empty keep limit, and direct-map threshold;
- `ReleaseDecision`: cache, return to central pool, purge, unmap, or quarantine, plus poison/redzone, remote-queue, cache-limit, and reclaim-limit fields.

Modes do not directly mutate page/span/mmap internals.

| Mode | AllocationPlan | ReleaseDecision | Current status |
|---|---|---|---|
| `Balanced` | `Auto`, reuse enabled, no TLS magazine, default empty keep limit | return to central pool with default keep limit | Stable shared-pool baseline |
| `ThroughputCache` | size-class/span preference, large thread-local magazine, batch refill, high empty keep limit | cache with high per-bin limits | Hot same-thread churn path implemented |
| `DeterministicLatency` | `Auto`, small bounded TLS magazine, fixed small batch, avoids aggressive release | cache with bounded per-bin limits | Stable hot-path reuse implemented; sampled p99 telemetry is future work |
| `CompactRSS` | low-RSS, no thread cache, occupancy packing, direct map only above compact threshold, keep limit 0 | targeted purge/unmap of the emptied page/span | Fine-grained empty page/span reclaim implemented |
| `FragmentationStable` | span/page storage with occupancy-aware packing and low keep limit; avoids turning medium objects into large direct maps | return to central pool with low keep limit; unmap only true direct mappings | Occupancy-aware reuse implemented; adaptive size-class table rebalance is future work |
| `CrossThreadMessage` | owner-aware remote-free queue, owner-side drain on cache miss, moderate TLS magazine | remote frees enqueue to owner; local frees cache | Remote-free queue implemented for pooled objects |
| `LargeObjectStreaming` | extent/direct-map path only for true large objects (`>=128 KiB`), small/medium stays in normal services, bounded large extent reuse cache | cache a small bounded direct-map extent ring, otherwise return/unmap | Large-object isolation no longer covers medium objects |
| `HardenedDebug` | direct map, no reuse, header canary, tail redzone, quarantine intent | quarantine with poison/redzone checks | Header checks, double-free/invalid-free counters, poison, quarantine retention, and tail redzone validation implemented |

## Runtime Telemetry and Selector

Files:

- `include/my_ptmalloc/adaptive_telemetry.h`
- `src/adaptive_telemetry.cpp`
- `include/my_ptmalloc/adaptive_selector.h`
- `src/adaptive_selector.cpp`
- `include/my_ptmalloc/adaptive_model_selector.h`
- `src/adaptive_model_selector.cpp`
- generated model: `include/my_ptmalloc/generated_selector_model.h`
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

The selector extracts these as delta-window features. Hot-path telemetry keeps cumulative counters for snapshots and JSON output, while `adaptive_extract_window_features()` advances an internal baseline and returns only the activity since the previous extraction. Gauge-style fields such as live bytes and mapped bytes remain current values so mapped/live pressure is evaluated against the allocator's actual retained memory.

At each selector window boundary, `src/adaptive_selector.cpp` records an
`AdaptiveSelectorEvent` into a fixed-size ring buffer. Each event stores the
previous/current/candidate mode, selector backend, rule candidate for explicit
rule-baseline runs, model candidate, model confidence, whether a soft switch
happened, the decision reason, and the window feature values used for the
decision. Tooling such as `bench_runner` reads this ring buffer for explanation;
it does not call `adaptive_extract_window_features()` from the Web snapshot path.

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

## Offline-Trained Model Selector

`MY_MALLOC_ADAPTIVE_MODE_SELECTOR=model` enables a lightweight cost model:

```text
WorkloadFeatures window
  -> evaluate generated cost model for all 8 AdaptiveMode candidates
  -> apply cooldown and hysteresis
  -> soft-switch only if expected gain is high enough
```

The model is trained offline by:

```bash
python3 tools/evaluate_selector_model.py \
  --json-out models/selector_evaluation_results.json \
  --markdown-out docs/adaptive_selector_model_results.md

python3 tools/train_selector_model.py \
  --dataset models/selector_evaluation_results.json \
  --model-out include/my_ptmalloc/generated_selector_model.h \
  --summary-out models/selector_training_summary.json
```

The training script does not contain a hand-written mode oracle. It reads real
benchmark measurements from `evaluate_selector_model.py`, learns objective
weights from pairwise fixed-mode outcomes, derives per-mode cost labels from
those learned weights, and emits a compact boosted-stump ensemble plus per-mode
cost biases into `generated_selector_model.h`. The generated header also stores
the learned `SWITCH_COST`. Runtime inference is just a few comparisons and
additions at selector window boundaries, not on every allocation. The training
summary records model id, feature names, mode names, objective weights,
switch-cost, learned-cost accuracy, mean regret, and per-mode pick counts.

See [adaptive_selector_model.md](adaptive_selector_model.md) for the model
architecture diagrams, training pipeline, generated header format, runtime
selector flow, Web telemetry fields, and future trace-training plan.

`MY_MALLOC_ADAPTIVE_MODE=auto` defaults to the measured model selector. The
model path does not call the legacy rule selector and does not use rule fallback.
`MY_MALLOC_ADAPTIVE_MODE_SELECTOR=rule` remains available only as an explicit
debug/baseline mode for comparing against the older rule implementation.

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
| `MY_MALLOC_ADAPTIVE_MODE_SELECTOR` | `model`, `rule` baseline, `fixed`, `manual` |
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
- `selector_last_window.selector_backend`, `model_candidate`,
  `model_confidence`, and legacy `rule_candidate` when the explicit rule
  baseline is enabled.

## Current Limitations

Implemented:

- split shared memory services / mode policy / telemetry selector files;
- allocation-time `mode_id`;
- shared ownership table and registry;
- size-class pages, spans, direct mappings;
- thread-local cache bins for size-class pages and spans;
- owner-keyed remote-free queues with owner-side drain;
- compact RSS targeted reclaim of the exact empty page/span on free;
- fragmentation-stable occupancy-aware page/span reuse;
- hardened debug header cookie and tail redzone validation;
- per-mode and per-storage stats;
- remote-free telemetry;
- legacy rule selector for baseline comparison;
- offline-trained compact model selector as the default auto selector;
- delta-window feature extraction for selector decisions;
- compact RSS purge/unmap behavior;
- true-large extent/direct-map isolation with bounded reuse;
- hardened debug poison and quarantine mapping retention.

Future work:

- dynamic tcache sizing and deeper batch drain tuning;
- larger remote-free table and abandoned-owner cleanup;
- fragmentation-stable adaptive size-class table rebalancing beyond the current occupancy-aware packing;
- sampled p95/p99 latency;
- front redzone/page-guard debug variants;
- configurable multi-window smoothing for noisy workloads.
- collecting more measured trace labels from external applications, not only
  generated workloads.

## Commands

```bash
./build/allocator_validate --strategy adaptive
MY_MALLOC_ADAPTIVE_MODE=compact_rss ./build/allocator_validate --strategy adaptive
MY_MALLOC_ADAPTIVE_MODE=auto MY_MALLOC_ADAPTIVE_MODE_WINDOW=64 ./build/bench_runner --strategy adaptive --bench cross_thread_free --json
MY_MALLOC_ADAPTIVE_MODE=large_object ./build/bench_runner --strategy adaptive --bench phase_changing --json
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=adaptive ./build/test_basic
```
