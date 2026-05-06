# Adaptive Multi-Mode Allocator

`MY_MALLOC_MODE=adaptive` selects the project's independent adaptive allocator. It is not a dispatcher over the teaching allocators. `adaptive_demo` / `demo_all` keep that old teaching-dispatcher demonstration role.

The top-level adaptive abstraction is `AdaptiveMode`. Pooled page/span storage and direct mapping are internal helper paths only.

## 1. Shared Runtime

All adaptive modes share one runtime:

- `AdaptiveHeader` before every allocation;
- ownership page table and optional debug registry lookup;
- page/span/region metadata;
- mode table and soft-switch state;
- shared stats and workload telemetry;
- safe free routing.

`AdaptiveHeader` records `mode_id` at allocation time. `malloc` uses the current active mode, while `free`, `realloc`, and `usable_size` use the header's `mode_id`. A mode switch affects future allocations only. Live objects are not migrated.

## 2. Adaptive Modes

| Mode | Best fit | Implemented in phase 1 | Cost / limitation |
|---|---|---|---|
| `Balanced` | Unknown or mixed workloads | Conservative small/medium/large helper routing and default empty-page retention | Not specialized |
| `ThroughputCache` | Local malloc/free, server/RPC style churn | Keeps more empty pages/spans and uses cache-heavy helper routing | Higher RSS; real TLS cache is still future work |
| `DeterministicLatency` | Low p99 jitter workloads | Avoids aggressive pooled release and keeps stable pooled reuse | More retained memory |
| `CompactRSS` | Memory-constrained or peak-then-free workloads | Aggressively releases empty pages/spans | More mmap/refill activity |
| `FragmentationStable` | Long mixed-size services | Tracks entropy, usable/requested, mapped/live and uses conservative pooled reuse | Size-class policy is still prototype |
| `CrossThreadMessage` | Producer/consumer and remote-free workloads | Records owner thread and remote-free telemetry; reserves remote-free hooks | Remote-free queue is not implemented yet |
| `LargeObjectStreaming` | Large buffer bursts | Biases allocations above 4 KiB to direct mmap and returns them quickly | Can overuse mmap for medium objects |
| `HardenedDebug` | Tests, diagnostics, teaching allocator bugs | Direct-mmap bias, stronger header checks, invalid/double-free counters, poison-on-free | Quarantine/redzone/canary are TODO |

The mode table defines each mode's name, objective, storage-helper selector, free/realloc/usable handlers, and optional activation/retire hooks.

## 3. Soft Switching

Soft switching invariants:

- `adaptive_malloc` reads the active mode.
- Allocation metadata stores `mode_id`.
- `adaptive_free`, `adaptive_realloc`, and `adaptive_usable_size` dispatch through the allocation-time mode table entry.
- Old modes enter a retired/draining state and stop receiving new allocations.
- Retired modes still own live objects allocated under that mode.

Phase 1 implements active/previous/retired state and per-mode live/free stats. It does not yet reclaim all possible mode-local future state because most storage is still in shared helper pools.

## 4. Workload Feature Extraction

The selector uses coarse, real counters. Missing advanced metrics are kept as zero or TODO rather than invented.

Tracked now:

- size profile: histogram, entropy, large-bytes ratio;
- lifetime profile: alloc/free/realloc counts, live bytes;
- thread profile: owner thread token, same-thread free count, remote-free count, remote-free ratio;
- reuse/cache profile: pool hit/miss and reuse rate;
- memory profile: requested bytes, usable bytes, mapped bytes, mapped/live ratio, internal fragmentation estimate, empty/released pages/spans;
- latency profile: slow-path estimate from mmap plus pool misses;
- safety profile: invalid free, double free, header corruption counters.

Future work can add sampled p95/p99 latency, free burstiness, survival rate, per-thread imbalance, external fragmentation, quarantine hits, and maintenance-task counts.

## 5. Mode Selection

`MY_MALLOC_ADAPTIVE_MODE_SELECTOR=rule` enables the phase-1 rule baseline:

1. Debug flag forces `HardenedDebug`.
2. Severe mapped/live pressure selects `CompactRSS`.
3. High remote-free ratio selects `CrossThreadMessage`.
4. High large-bytes ratio selects `LargeObjectStreaming`.
5. High slow-path ratio selects `DeterministicLatency`.
6. High size entropy plus fragmentation selects `FragmentationStable`.
7. High local reuse selects `ThroughputCache`.
8. Otherwise use `Balanced`.

Safety guards:

- `MY_MALLOC_ADAPTIVE_MODE_WINDOW` controls observation cadence.
- `MY_MALLOC_ADAPTIVE_MODE_COOLDOWN` prevents window-to-window thrashing.
- Hysteresis is represented by expected-gain thresholds.
- Severe memory pressure can force `CompactRSS`.
- `MY_MALLOC_ADAPTIVE_DEBUG_MODE=1` forces `HardenedDebug`.

`MY_MALLOC_ADAPTIVE_MODE_SELECTOR=fixed` keeps the configured mode. `manual` lets code call `adaptive_set_mode()` for tests and controlled experiments.

## 6. Configuration

Primary mode configuration:

| Env var | Values |
|---|---|
| `MY_MALLOC_ADAPTIVE_MODE` | `balanced`, `throughput_cache`, `deterministic_latency`, `compact_rss`, `fragmentation_stable`, `cross_thread`, `large_object`, `hardened_debug`, `auto` |
| `MY_MALLOC_ADAPTIVE_MODE_SELECTOR` | `rule`, `fixed`, `manual` |
| `MY_MALLOC_ADAPTIVE_MODE_WINDOW` | Selector window size |
| `MY_MALLOC_ADAPTIVE_MODE_COOLDOWN` | Cooldown in windows |
| `MY_MALLOC_ADAPTIVE_DEBUG_MODE` | `0` / `1` |

## 7. Stats And Benchmark JSON

`AdaptiveStatsSnapshot` and `bench_runner --json` now expose mode-first fields:

- `current_mode`, `active_mode`, `previous_mode`;
- `mode_switches`, `retired_mode_count`;
- `mode_alloc_count`, `mode_free_count`;
- `mode_live_bytes`, `mode_mapped_bytes`;
- `remote_free_ratio`, `size_entropy`, `large_bytes_ratio`;
- `mapped_live_ratio`, `fragmentation_estimate`, `slow_path_ratio`;
- `double_free_count`, `invalid_free_count`.

## 8. Benchmark Scenarios

Useful scenarios for mode work:

- `throughput_server`: many short-lived local allocations; should favor `ThroughputCache`.
- `realtime_latency`: stable repeated allocation pattern; should favor `DeterministicLatency`.
- `memory_constrained`: peak then free; should favor `CompactRSS`.
- `producer_consumer`: one thread allocates and another frees; telemetry should select `CrossThreadMessage`.
- `large_streaming`: large-object bursts; should favor `LargeObjectStreaming`.
- `debug_safety`: invalid/double free checks; should favor `HardenedDebug`.

Phase 1 proves routing, telemetry, and obvious policy differences. It does not claim every mode is already performance-dominant.

## 9. Limitations

Complete enough in phase 1:

- mode id in allocation metadata;
- mode table dispatch for malloc/free/realloc/usable_size;
- soft-switch state;
- per-mode stats;
- owner-thread remote-free telemetry;
- rule selector with cooldown and gain guards;
- aggressive release behavior for `CompactRSS`;
- direct-mmap bias for `LargeObjectStreaming` and `HardenedDebug`.

Prototype/TODO:

- real thread-local cache and batch refill/drain;
- remote-free queue and owner-aware reclaim;
- fragmentation-stable size-class table policy;
- sampled p95/p99 latency;
- hardened quarantine, redzones, canaries, and richer diagnostics.

## 10. Commands

```bash
./build/allocator_validate --strategy adaptive
MY_MALLOC_ADAPTIVE_MODE=compact_rss ./build/allocator_validate --strategy adaptive
MY_MALLOC_ADAPTIVE_MODE=auto MY_MALLOC_ADAPTIVE_MODE_WINDOW=64 ./build/bench_runner --strategy adaptive --bench cross_thread_free --json
MY_MALLOC_ADAPTIVE_MODE=large_object ./build/bench_runner --strategy adaptive --bench phase_changing --json
LD_PRELOAD=./build/libmy_ptmalloc.so MY_MALLOC_MODE=adaptive ./build/test_basic
```
