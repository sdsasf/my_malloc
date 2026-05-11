# Adaptive Selector Model Evaluation Results

Generated at: `2026-05-11 21:15:15 +0800`

These measurements compare fixed adaptive modes against the measured-cost `model` selector on the generated workload suite. When requested, the legacy `rule` selector is included only as a baseline. Lower score is better.

## Reproduction Command

```bash
python3 tools/evaluate_selector_model.py --bench-runner ./build/bench_runner --iters 20000 --slots 512 --threads 2 --seeds 10001 10002 10003 10004 10005 10006 10007 10008 10009 10010 10011 10012 10013 10014 10015 10016 10017 10018 10019 10020 --mode-window 64 --mode-cooldown 0 --json-out models/selector_training_dataset.json --markdown-out docs/adaptive_selector_training_dataset.md --fixed-only
```

## Score Definition

Score is normalized per workload across all compared cases. The metric weights are loaded from the trained objective model summary, so the report uses the same learned objective family as training:

| Metric | Weight | Direction |
|---|---:|---|
| `fragmentation_estimate` | 0.12 | lower is better |
| `mapped_live_ratio` | 0.18 | lower is better |
| `mode_switches` | 0.05 | lower is better |
| `ms` | 0.08 | lower is better |
| `peak_rss_kb` | 0.39 | lower is better |
| `slow_path_ratio` | 0.13 | lower is better |
| `validation_errors` | 0.05 | lower is better |

## Overall Ranking

| Rank | Case | Mean score | Mean ops/sec | Mean peak RSS KB | Mean switches | Best workload count |
|---:|---|---:|---:|---:|---:|---:|
| 1 | `fixed:compact_rss` | 0.232 | 1037040 | 28917 | 0.0 | 57 |
| 2 | `fixed:balanced` | 0.272 | 2268078 | 30962 | 0.0 | 20 |
| 3 | `fixed:fragmentation_stable` | 0.283 | 2376399 | 30646 | 0.0 | 22 |
| 4 | `fixed:large_object` | 0.289 | 2369005 | 30528 | 0.0 | 18 |
| 5 | `fixed:deterministic_latency` | 0.328 | 2486610 | 31238 | 0.0 | 6 |
| 6 | `fixed:throughput_cache` | 0.377 | 2486947 | 31520 | 0.0 | 10 |
| 7 | `fixed:cross_thread` | 0.388 | 2522900 | 31436 | 0.0 | 7 |

## Per-Workload Winners

| Workload | Best overall | Best fixed mode | Model selector |
|---|---|---|---|
| `adaptive_mix:seed10001` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10002` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10003` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10004` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10005` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10006` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10007` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10008` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10009` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10010` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10011` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10012` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10013` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10014` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10015` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10016` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10017` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10018` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10019` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `adaptive_mix:seed10020` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10001` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10002` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10003` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10004` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10005` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10006` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10007` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10008` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10009` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10010` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10011` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10012` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10013` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10014` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10015` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10016` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10017` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10018` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10019` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10020` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `large_burst:seed10001` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10002` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10003` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `large_burst:seed10004` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10005` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `large_burst:seed10006` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10007` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `large_burst:seed10008` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `large_burst:seed10009` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10010` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10011` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `large_burst:seed10012` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `large_burst:seed10013` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10014` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10015` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10016` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `large_burst:seed10017` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10018` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `large_burst:seed10019` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10020` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `latency_loop:seed10001` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10002` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10003` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10004` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10005` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `latency_loop:seed10006` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10007` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10008` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10009` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10010` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10011` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `latency_loop:seed10012` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10013` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10014` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10015` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `latency_loop:seed10016` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10017` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10018` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10019` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10020` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `remote_queue:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10002` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10003` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10004` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10005` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10006` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10007` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10008` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10009` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10010` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10011` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10012` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10013` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10014` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10015` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10016` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10017` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10018` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10020` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10001` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10002` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10003` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10004` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10005` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10006` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10007` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10008` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10009` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10010` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10011` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10012` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10014` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10015` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10016` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10017` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10018` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10020` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10001` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10002` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `throughput_churn:seed10003` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `throughput_churn:seed10004` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10005` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `throughput_churn:seed10006` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `throughput_churn:seed10007` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `throughput_churn:seed10008` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `throughput_churn:seed10009` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `throughput_churn:seed10010` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `throughput_churn:seed10011` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10012` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `throughput_churn:seed10013` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `throughput_churn:seed10014` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10016` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10017` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `throughput_churn:seed10018` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10019` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10020` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |

## `adaptive_mix:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 79258 | 35528 | `compact_rss` | 0 | 109.39 | 0.005 | 0.978 |
| `fixed:large_object` | 0.393 | 187240 | 42128 | `large_object` | 0 | 3944.83 | 0.005 | 0.408 |
| `fixed:fragmentation_stable` | 0.401 | 202669 | 42288 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.444 |
| `fixed:balanced` | 0.498 | 196940 | 43496 | `balanced` | 0 | 5639.60 | 0.005 | 0.440 |
| `fixed:deterministic_latency` | 0.550 | 221367 | 44160 | `deterministic_latency` | 0 | 6855.03 | 0.005 | 0.440 |
| `fixed:cross_thread` | 0.563 | 209456 | 44160 | `cross_thread` | 0 | 7268.27 | 0.005 | 0.444 |
| `fixed:throughput_cache` | 0.577 | 207311 | 44160 | `throughput_cache` | 0 | 7790.91 | 0.005 | 0.449 |

## `adaptive_mix:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 72204 | 37168 | `compact_rss` | 0 | 109.39 | 0.005 | 0.987 |
| `fixed:large_object` | 0.362 | 217328 | 43040 | `large_object` | 0 | 3945.59 | 0.005 | 0.403 |
| `fixed:fragmentation_stable` | 0.373 | 197440 | 42992 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.454 |
| `fixed:balanced` | 0.486 | 206762 | 44624 | `balanced` | 0 | 5590.98 | 0.005 | 0.448 |
| `fixed:deterministic_latency` | 0.556 | 196048 | 45440 | `deterministic_latency` | 0 | 6855.03 | 0.005 | 0.448 |
| `fixed:cross_thread` | 0.560 | 206854 | 45312 | `cross_thread` | 0 | 7365.51 | 0.005 | 0.452 |
| `fixed:throughput_cache` | 0.567 | 203948 | 45184 | `throughput_cache` | 0 | 7827.37 | 0.005 | 0.457 |

## `adaptive_mix:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 76903 | 38428 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:large_object` | 0.387 | 211424 | 43860 | `large_object` | 0 | 3949.39 | 0.004 | 0.402 |
| `fixed:fragmentation_stable` | 0.407 | 191712 | 43956 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.458 |
| `fixed:balanced` | 0.477 | 208583 | 44592 | `balanced` | 0 | 5590.98 | 0.004 | 0.454 |
| `fixed:deterministic_latency` | 0.553 | 205843 | 45440 | `deterministic_latency` | 0 | 6855.03 | 0.004 | 0.454 |
| `fixed:cross_thread` | 0.567 | 193773 | 45440 | `cross_thread` | 0 | 7304.74 | 0.004 | 0.457 |
| `fixed:throughput_cache` | 0.578 | 206420 | 45440 | `throughput_cache` | 0 | 7875.99 | 0.004 | 0.462 |

## `adaptive_mix:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 77097 | 37420 | `compact_rss` | 0 | 109.39 | 0.005 | 0.991 |
| `fixed:large_object` | 0.407 | 216478 | 44352 | `large_object` | 0 | 3944.07 | 0.005 | 0.408 |
| `fixed:fragmentation_stable` | 0.414 | 206667 | 44288 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.455 |
| `fixed:balanced` | 0.499 | 198194 | 45184 | `balanced` | 0 | 5688.21 | 0.005 | 0.451 |
| `fixed:cross_thread` | 0.547 | 210978 | 45440 | `cross_thread` | 0 | 7341.20 | 0.005 | 0.456 |
| `fixed:deterministic_latency` | 0.552 | 207421 | 45824 | `deterministic_latency` | 0 | 6806.41 | 0.005 | 0.451 |
| `fixed:throughput_cache` | 0.567 | 201886 | 45568 | `throughput_cache` | 0 | 7827.37 | 0.005 | 0.460 |

## `adaptive_mix:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 77889 | 36920 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:large_object` | 0.397 | 199083 | 43044 | `large_object` | 0 | 3946.35 | 0.004 | 0.404 |
| `fixed:fragmentation_stable` | 0.413 | 207177 | 43232 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.455 |
| `fixed:balanced` | 0.508 | 201516 | 44288 | `balanced` | 0 | 5639.60 | 0.004 | 0.451 |
| `fixed:deterministic_latency` | 0.546 | 214269 | 44544 | `deterministic_latency` | 0 | 6903.64 | 0.004 | 0.451 |
| `fixed:cross_thread` | 0.565 | 208356 | 44672 | `cross_thread` | 0 | 7316.89 | 0.004 | 0.455 |
| `fixed:throughput_cache` | 0.570 | 212847 | 44544 | `throughput_cache` | 0 | 7839.52 | 0.004 | 0.460 |

## `adaptive_mix:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 75702 | 37464 | `compact_rss` | 0 | 109.39 | 0.004 | 0.989 |
| `fixed:large_object` | 0.388 | 216153 | 42848 | `large_object` | 0 | 3944.83 | 0.004 | 0.402 |
| `fixed:fragmentation_stable` | 0.428 | 186260 | 43264 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.456 |
| `fixed:balanced` | 0.486 | 195034 | 43648 | `balanced` | 0 | 5590.98 | 0.004 | 0.452 |
| `fixed:deterministic_latency` | 0.557 | 197053 | 44416 | `deterministic_latency` | 0 | 6806.41 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.564 | 193433 | 44288 | `cross_thread` | 0 | 7353.35 | 0.004 | 0.456 |
| `fixed:throughput_cache` | 0.573 | 200104 | 44288 | `throughput_cache` | 0 | 7778.75 | 0.004 | 0.460 |

## `adaptive_mix:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 85309 | 37108 | `compact_rss` | 0 | 109.39 | 0.004 | 0.986 |
| `fixed:large_object` | 0.398 | 257861 | 42808 | `large_object` | 0 | 3945.59 | 0.004 | 0.405 |
| `fixed:fragmentation_stable` | 0.421 | 220421 | 42940 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.453 |
| `fixed:balanced` | 0.517 | 205186 | 43904 | `balanced` | 0 | 5590.98 | 0.004 | 0.448 |
| `fixed:deterministic_latency` | 0.557 | 200321 | 44160 | `deterministic_latency` | 0 | 6611.94 | 0.004 | 0.448 |
| `fixed:cross_thread` | 0.570 | 228030 | 44288 | `cross_thread` | 0 | 7110.27 | 0.004 | 0.452 |
| `fixed:throughput_cache` | 0.588 | 196433 | 44288 | `throughput_cache` | 0 | 7523.51 | 0.004 | 0.456 |

## `adaptive_mix:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 79569 | 37576 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:large_object` | 0.372 | 249219 | 43452 | `large_object` | 0 | 3947.87 | 0.004 | 0.401 |
| `fixed:fragmentation_stable` | 0.402 | 213898 | 43724 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.464 |
| `fixed:balanced` | 0.510 | 213020 | 45056 | `balanced` | 0 | 5736.83 | 0.004 | 0.460 |
| `fixed:deterministic_latency` | 0.564 | 194427 | 45568 | `deterministic_latency` | 0 | 6855.03 | 0.004 | 0.460 |
| `fixed:cross_thread` | 0.565 | 227312 | 45440 | `cross_thread` | 0 | 7426.28 | 0.004 | 0.464 |
| `fixed:throughput_cache` | 0.583 | 196609 | 45440 | `throughput_cache` | 0 | 7888.14 | 0.004 | 0.468 |

## `adaptive_mix:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 81481 | 38860 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:large_object` | 0.362 | 239809 | 44256 | `large_object` | 0 | 3945.59 | 0.004 | 0.417 |
| `fixed:fragmentation_stable` | 0.376 | 217176 | 44276 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.465 |
| `fixed:balanced` | 0.482 | 214147 | 45576 | `balanced` | 0 | 5590.98 | 0.004 | 0.461 |
| `fixed:deterministic_latency` | 0.543 | 217563 | 46208 | `deterministic_latency` | 0 | 6855.03 | 0.004 | 0.461 |
| `fixed:cross_thread` | 0.550 | 233298 | 46208 | `cross_thread` | 0 | 7292.58 | 0.004 | 0.464 |
| `fixed:throughput_cache` | 0.580 | 218060 | 46464 | `throughput_cache` | 0 | 7827.37 | 0.004 | 0.468 |

## `adaptive_mix:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 90535 | 37516 | `compact_rss` | 0 | 109.39 | 0.004 | 0.996 |
| `fixed:large_object` | 0.386 | 264225 | 43652 | `large_object` | 0 | 3950.91 | 0.004 | 0.404 |
| `fixed:fragmentation_stable` | 0.415 | 229225 | 43904 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.461 |
| `fixed:balanced` | 0.481 | 235551 | 44472 | `balanced` | 0 | 5590.98 | 0.004 | 0.457 |
| `fixed:deterministic_latency` | 0.561 | 224285 | 45440 | `deterministic_latency` | 0 | 6952.26 | 0.004 | 0.457 |
| `fixed:cross_thread` | 0.563 | 240981 | 45312 | `cross_thread` | 0 | 7414.12 | 0.004 | 0.461 |
| `fixed:throughput_cache` | 0.574 | 247548 | 45312 | `throughput_cache` | 0 | 7924.60 | 0.004 | 0.466 |

## `adaptive_mix:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 88641 | 36812 | `compact_rss` | 0 | 109.39 | 0.004 | 0.984 |
| `fixed:large_object` | 0.407 | 232022 | 42572 | `large_object` | 0 | 3944.07 | 0.004 | 0.406 |
| `fixed:fragmentation_stable` | 0.419 | 218242 | 42584 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.452 |
| `fixed:balanced` | 0.506 | 216747 | 43392 | `balanced` | 0 | 5688.21 | 0.004 | 0.448 |
| `fixed:cross_thread` | 0.554 | 221286 | 43520 | `cross_thread` | 0 | 7499.20 | 0.004 | 0.452 |
| `fixed:deterministic_latency` | 0.560 | 206404 | 43776 | `deterministic_latency` | 0 | 7000.88 | 0.004 | 0.448 |
| `fixed:throughput_cache` | 0.563 | 225505 | 43520 | `throughput_cache` | 0 | 7875.99 | 0.004 | 0.456 |

## `adaptive_mix:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 75151 | 38024 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:large_object` | 0.334 | 194314 | 43352 | `large_object` | 0 | 3944.83 | 0.004 | 0.426 |
| `fixed:fragmentation_stable` | 0.362 | 202407 | 43864 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.463 |
| `fixed:balanced` | 0.482 | 184358 | 45508 | `balanced` | 0 | 5639.60 | 0.004 | 0.458 |
| `fixed:deterministic_latency` | 0.549 | 196668 | 46464 | `deterministic_latency` | 0 | 6806.41 | 0.004 | 0.458 |
| `fixed:cross_thread` | 0.562 | 180938 | 46336 | `cross_thread` | 0 | 7353.35 | 0.004 | 0.462 |
| `fixed:throughput_cache` | 0.570 | 193924 | 46336 | `throughput_cache` | 0 | 7815.22 | 0.004 | 0.467 |

## `adaptive_mix:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 74360 | 37140 | `compact_rss` | 0 | 109.39 | 0.004 | 0.998 |
| `fixed:large_object` | 0.371 | 160518 | 42604 | `large_object` | 0 | 3947.11 | 0.004 | 0.404 |
| `fixed:fragmentation_stable` | 0.415 | 145510 | 43124 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.467 |
| `fixed:balanced` | 0.470 | 187698 | 43764 | `balanced` | 0 | 5590.98 | 0.004 | 0.462 |
| `fixed:deterministic_latency` | 0.557 | 188028 | 44928 | `deterministic_latency` | 0 | 6855.03 | 0.004 | 0.462 |
| `fixed:throughput_cache` | 0.572 | 194669 | 44800 | `throughput_cache` | 0 | 7778.75 | 0.004 | 0.470 |
| `fixed:cross_thread` | 0.576 | 148442 | 44800 | `cross_thread` | 0 | 7304.74 | 0.004 | 0.466 |

## `adaptive_mix:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 68863 | 36200 | `compact_rss` | 0 | 109.39 | 0.004 | 0.983 |
| `fixed:large_object` | 0.414 | 212647 | 42012 | `large_object` | 0 | 3945.59 | 0.004 | 0.394 |
| `fixed:fragmentation_stable` | 0.430 | 201252 | 42080 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.449 |
| `fixed:balanced` | 0.506 | 179418 | 42624 | `balanced` | 0 | 5590.98 | 0.004 | 0.445 |
| `fixed:deterministic_latency` | 0.548 | 199930 | 43008 | `deterministic_latency` | 0 | 6660.56 | 0.004 | 0.445 |
| `fixed:cross_thread` | 0.568 | 199272 | 43136 | `cross_thread` | 0 | 7171.04 | 0.004 | 0.449 |
| `fixed:throughput_cache` | 0.582 | 188846 | 43136 | `throughput_cache` | 0 | 7620.75 | 0.004 | 0.453 |

## `adaptive_mix:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 76864 | 35964 | `compact_rss` | 0 | 109.39 | 0.005 | 0.983 |
| `fixed:large_object` | 0.392 | 226758 | 42192 | `large_object` | 0 | 3944.83 | 0.005 | 0.395 |
| `fixed:fragmentation_stable` | 0.414 | 206355 | 42356 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.450 |
| `fixed:balanced` | 0.507 | 221828 | 43492 | `balanced` | 0 | 5639.60 | 0.005 | 0.445 |
| `fixed:deterministic_latency` | 0.546 | 226210 | 43776 | `deterministic_latency` | 0 | 6757.79 | 0.005 | 0.445 |
| `fixed:cross_thread` | 0.570 | 207253 | 43904 | `cross_thread` | 0 | 7304.74 | 0.005 | 0.450 |
| `fixed:throughput_cache` | 0.571 | 222770 | 43776 | `throughput_cache` | 0 | 7730.14 | 0.005 | 0.453 |

## `adaptive_mix:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 75732 | 37504 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:large_object` | 0.379 | 211474 | 42856 | `large_object` | 0 | 3944.07 | 0.004 | 0.406 |
| `fixed:fragmentation_stable` | 0.412 | 204040 | 43280 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.457 |
| `fixed:balanced` | 0.493 | 195131 | 43984 | `balanced` | 0 | 5590.98 | 0.004 | 0.452 |
| `fixed:deterministic_latency` | 0.557 | 192944 | 44672 | `deterministic_latency` | 0 | 6709.18 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.563 | 212611 | 44672 | `cross_thread` | 0 | 7134.58 | 0.004 | 0.456 |
| `fixed:throughput_cache` | 0.571 | 202959 | 44544 | `throughput_cache` | 0 | 7657.21 | 0.004 | 0.460 |

## `adaptive_mix:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 75564 | 36524 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:large_object` | 0.382 | 213639 | 42532 | `large_object` | 0 | 3946.35 | 0.004 | 0.419 |
| `fixed:fragmentation_stable` | 0.408 | 190741 | 42828 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.457 |
| `fixed:balanced` | 0.502 | 210036 | 44032 | `balanced` | 0 | 5590.98 | 0.004 | 0.453 |
| `fixed:cross_thread` | 0.553 | 212647 | 44288 | `cross_thread` | 0 | 7171.04 | 0.004 | 0.456 |
| `fixed:deterministic_latency` | 0.557 | 196220 | 44544 | `deterministic_latency` | 0 | 6709.18 | 0.004 | 0.452 |
| `fixed:throughput_cache` | 0.567 | 221092 | 44416 | `throughput_cache` | 0 | 7559.98 | 0.004 | 0.461 |

## `adaptive_mix:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 75757 | 37220 | `compact_rss` | 0 | 109.39 | 0.004 | 0.981 |
| `fixed:large_object` | 0.388 | 220749 | 42896 | `large_object` | 0 | 3945.59 | 0.004 | 0.397 |
| `fixed:fragmentation_stable` | 0.433 | 179592 | 43376 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.448 |
| `fixed:balanced` | 0.498 | 221378 | 44032 | `balanced` | 0 | 5639.60 | 0.004 | 0.444 |
| `fixed:cross_thread` | 0.556 | 201391 | 44288 | `cross_thread` | 0 | 7341.20 | 0.004 | 0.447 |
| `fixed:deterministic_latency` | 0.558 | 206690 | 44544 | `deterministic_latency` | 0 | 6903.64 | 0.004 | 0.444 |
| `fixed:throughput_cache` | 0.565 | 210333 | 44288 | `throughput_cache` | 0 | 7754.44 | 0.004 | 0.452 |

## `adaptive_mix:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 71122 | 37548 | `compact_rss` | 0 | 109.39 | 0.004 | 0.989 |
| `fixed:large_object` | 0.392 | 196594 | 43052 | `large_object` | 0 | 3947.11 | 0.004 | 0.406 |
| `fixed:fragmentation_stable` | 0.400 | 186516 | 43008 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.455 |
| `fixed:balanced` | 0.493 | 195115 | 43996 | `balanced` | 0 | 5639.60 | 0.004 | 0.450 |
| `fixed:deterministic_latency` | 0.546 | 186666 | 44416 | `deterministic_latency` | 0 | 6855.03 | 0.004 | 0.450 |
| `fixed:cross_thread` | 0.563 | 192238 | 44544 | `cross_thread` | 0 | 7316.89 | 0.004 | 0.454 |
| `fixed:throughput_cache` | 0.571 | 187752 | 44416 | `throughput_cache` | 0 | 7875.99 | 0.004 | 0.459 |

## `adaptive_mix:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.214 | 75483 | 36080 | `compact_rss` | 0 | 109.39 | 0.005 | 0.985 |
| `fixed:large_object` | 0.377 | 201173 | 41752 | `large_object` | 0 | 3945.59 | 0.005 | 0.404 |
| `fixed:fragmentation_stable` | 0.400 | 200092 | 42052 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.451 |
| `fixed:balanced` | 0.489 | 189640 | 42968 | `balanced` | 0 | 5590.98 | 0.005 | 0.446 |
| `fixed:deterministic_latency` | 0.554 | 202323 | 43776 | `deterministic_latency` | 0 | 6757.79 | 0.005 | 0.446 |
| `fixed:cross_thread` | 0.564 | 212923 | 43776 | `cross_thread` | 0 | 7268.27 | 0.005 | 0.450 |
| `fixed:throughput_cache` | 0.574 | 190513 | 43648 | `throughput_cache` | 0 | 7693.67 | 0.005 | 0.454 |

## `fragmentation_drift:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.204 | 117309 | 46720 | `compact_rss` | 0 | 108.86 | 0.002 | 9.483 |
| `fixed:large_object` | 0.395 | 127400 | 52768 | `large_object` | 0 | 3695.41 | 0.002 | 8.625 |
| `fixed:fragmentation_stable` | 0.595 | 117687 | 53056 | `fragmentation_stable` | 0 | 3578.90 | 0.002 | 9.423 |
| `fixed:balanced` | 0.696 | 119824 | 54796 | `balanced` | 0 | 4721.97 | 0.002 | 9.423 |
| `fixed:cross_thread` | 0.714 | 123203 | 55552 | `cross_thread` | 0 | 4994.13 | 0.002 | 9.423 |
| `fixed:deterministic_latency` | 0.758 | 116675 | 55424 | `deterministic_latency` | 0 | 4994.13 | 0.002 | 9.423 |
| `fixed:throughput_cache` | 0.769 | 115956 | 55552 | `throughput_cache` | 0 | 4994.13 | 0.002 | 9.423 |

## `fragmentation_drift:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.160 | 113591 | 47216 | `compact_rss` | 0 | 108.86 | 0.002 | 9.685 |
| `fixed:large_object` | 0.342 | 119513 | 52364 | `large_object` | 0 | 3680.96 | 0.002 | 8.628 |
| `fixed:fragmentation_stable` | 0.494 | 118957 | 53120 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.625 |
| `fixed:cross_thread` | 0.676 | 121374 | 56192 | `cross_thread` | 0 | 5415.97 | 0.002 | 9.625 |
| `fixed:balanced` | 0.700 | 101666 | 55060 | `balanced` | 0 | 5034.95 | 0.002 | 9.625 |
| `fixed:deterministic_latency` | 0.719 | 112996 | 56448 | `deterministic_latency` | 0 | 5415.97 | 0.002 | 9.625 |
| `fixed:throughput_cache` | 0.738 | 106311 | 56192 | `throughput_cache` | 0 | 5415.97 | 0.002 | 9.625 |

## `fragmentation_drift:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.175 | 105109 | 47720 | `compact_rss` | 0 | 108.86 | 0.002 | 9.668 |
| `fixed:large_object` | 0.339 | 124505 | 52388 | `large_object` | 0 | 3640.98 | 0.002 | 8.521 |
| `fixed:fragmentation_stable` | 0.515 | 116323 | 53188 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.630 |
| `fixed:balanced` | 0.682 | 92487 | 54432 | `balanced` | 0 | 4721.97 | 0.002 | 9.630 |
| `fixed:cross_thread` | 0.713 | 113695 | 56064 | `cross_thread` | 0 | 5211.85 | 0.002 | 9.630 |
| `fixed:deterministic_latency` | 0.736 | 102632 | 55936 | `deterministic_latency` | 0 | 5211.85 | 0.002 | 9.630 |
| `fixed:throughput_cache` | 0.737 | 102137 | 55936 | `throughput_cache` | 0 | 5211.85 | 0.002 | 9.630 |

## `fragmentation_drift:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.199 | 117520 | 47488 | `compact_rss` | 0 | 108.86 | 0.002 | 9.430 |
| `fixed:large_object` | 0.403 | 125544 | 53852 | `large_object` | 0 | 3749.00 | 0.002 | 8.477 |
| `fixed:fragmentation_stable` | 0.618 | 115840 | 54244 | `fragmentation_stable` | 0 | 3633.33 | 0.002 | 9.343 |
| `fixed:balanced` | 0.665 | 121740 | 55628 | `balanced` | 0 | 4776.40 | 0.002 | 9.343 |
| `fixed:throughput_cache` | 0.710 | 120916 | 56320 | `throughput_cache` | 0 | 4994.13 | 0.002 | 9.343 |
| `fixed:deterministic_latency` | 0.713 | 121829 | 56576 | `deterministic_latency` | 0 | 4994.13 | 0.002 | 9.343 |
| `fixed:cross_thread` | 0.737 | 117232 | 56192 | `cross_thread` | 0 | 4994.13 | 0.002 | 9.343 |

## `fragmentation_drift:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.150 | 113000 | 47036 | `compact_rss` | 0 | 108.86 | 0.002 | 9.645 |
| `fixed:large_object` | 0.399 | 119828 | 52888 | `large_object` | 0 | 3722.63 | 0.002 | 8.511 |
| `fixed:fragmentation_stable` | 0.543 | 114809 | 53192 | `fragmentation_stable` | 0 | 3606.11 | 0.002 | 9.558 |
| `fixed:cross_thread` | 0.695 | 114013 | 55296 | `cross_thread` | 0 | 5075.77 | 0.002 | 9.558 |
| `fixed:balanced` | 0.709 | 95522 | 54384 | `balanced` | 0 | 4749.18 | 0.002 | 9.558 |
| `fixed:deterministic_latency` | 0.715 | 109517 | 55424 | `deterministic_latency` | 0 | 5075.77 | 0.002 | 9.558 |
| `fixed:throughput_cache` | 0.725 | 104920 | 55296 | `throughput_cache` | 0 | 5075.77 | 0.002 | 9.558 |

## `fragmentation_drift:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.162 | 111225 | 47456 | `compact_rss` | 0 | 108.86 | 0.002 | 9.698 |
| `fixed:large_object` | 0.390 | 114183 | 52524 | `large_object` | 0 | 3599.31 | 0.002 | 8.570 |
| `fixed:fragmentation_stable` | 0.532 | 114977 | 53140 | `fragmentation_stable` | 0 | 3483.64 | 0.002 | 9.634 |
| `fixed:deterministic_latency` | 0.686 | 115610 | 55296 | `deterministic_latency` | 0 | 4898.87 | 0.002 | 9.634 |
| `fixed:balanced` | 0.696 | 104918 | 54016 | `balanced` | 0 | 4572.28 | 0.002 | 9.634 |
| `fixed:throughput_cache` | 0.719 | 110411 | 55168 | `throughput_cache` | 0 | 4898.87 | 0.002 | 9.634 |
| `fixed:cross_thread` | 0.758 | 106385 | 55296 | `cross_thread` | 0 | 4898.87 | 0.002 | 9.634 |

## `fragmentation_drift:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.165 | 131465 | 48036 | `compact_rss` | 0 | 108.86 | 0.002 | 9.802 |
| `fixed:large_object` | 0.378 | 137071 | 53144 | `large_object` | 0 | 3627.38 | 0.002 | 8.732 |
| `fixed:fragmentation_stable` | 0.535 | 138021 | 54120 | `fragmentation_stable` | 0 | 3510.86 | 0.002 | 9.723 |
| `fixed:throughput_cache` | 0.683 | 137028 | 56064 | `throughput_cache` | 0 | 4871.65 | 0.002 | 9.723 |
| `fixed:balanced` | 0.707 | 128587 | 55680 | `balanced` | 0 | 4762.79 | 0.002 | 9.723 |
| `fixed:cross_thread` | 0.709 | 132239 | 56064 | `cross_thread` | 0 | 4871.65 | 0.002 | 9.723 |
| `fixed:deterministic_latency` | 0.769 | 123082 | 56192 | `deterministic_latency` | 0 | 4871.65 | 0.002 | 9.723 |

## `fragmentation_drift:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.194 | 122636 | 47100 | `compact_rss` | 0 | 108.86 | 0.002 | 9.642 |
| `fixed:large_object` | 0.370 | 139263 | 52760 | `large_object` | 0 | 3669.05 | 0.002 | 8.300 |
| `fixed:fragmentation_stable` | 0.548 | 133047 | 53608 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.585 |
| `fixed:balanced` | 0.656 | 134712 | 55120 | `balanced` | 0 | 4912.48 | 0.002 | 9.585 |
| `fixed:throughput_cache` | 0.692 | 138276 | 56064 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.585 |
| `fixed:deterministic_latency` | 0.717 | 131181 | 56064 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.585 |
| `fixed:cross_thread` | 0.767 | 118024 | 55936 | `cross_thread` | 0 | 5130.21 | 0.002 | 9.585 |

## `fragmentation_drift:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.199 | 135612 | 47584 | `compact_rss` | 0 | 108.86 | 0.002 | 9.640 |
| `fixed:large_object` | 0.354 | 148169 | 52824 | `large_object` | 0 | 3640.13 | 0.002 | 8.649 |
| `fixed:fragmentation_stable` | 0.537 | 140455 | 53372 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.583 |
| `fixed:balanced` | 0.666 | 137521 | 55020 | `balanced` | 0 | 4667.53 | 0.002 | 9.583 |
| `fixed:throughput_cache` | 0.721 | 140582 | 56320 | `throughput_cache` | 0 | 5102.99 | 0.002 | 9.583 |
| `fixed:cross_thread` | 0.750 | 136573 | 56448 | `cross_thread` | 0 | 5102.99 | 0.002 | 9.583 |
| `fixed:deterministic_latency` | 0.760 | 133048 | 56192 | `deterministic_latency` | 0 | 5102.99 | 0.002 | 9.583 |

## `fragmentation_drift:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.184 | 114681 | 47360 | `compact_rss` | 0 | 108.86 | 0.002 | 9.672 |
| `fixed:large_object` | 0.380 | 124478 | 53040 | `large_object` | 0 | 3724.33 | 0.002 | 8.498 |
| `fixed:fragmentation_stable` | 0.559 | 119305 | 53820 | `fragmentation_stable` | 0 | 3606.11 | 0.002 | 9.596 |
| `fixed:balanced` | 0.655 | 118011 | 54896 | `balanced` | 0 | 4749.18 | 0.002 | 9.596 |
| `fixed:deterministic_latency` | 0.742 | 113289 | 55936 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.596 |
| `fixed:cross_thread` | 0.756 | 112024 | 56064 | `cross_thread` | 0 | 5130.21 | 0.002 | 9.596 |
| `fixed:throughput_cache` | 0.764 | 109774 | 55936 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.596 |

## `fragmentation_drift:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.163 | 112669 | 46684 | `compact_rss` | 0 | 108.86 | 0.002 | 9.421 |
| `fixed:large_object` | 0.361 | 117076 | 51916 | `large_object` | 0 | 3640.13 | 0.002 | 8.498 |
| `fixed:fragmentation_stable` | 0.524 | 113827 | 52344 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.375 |
| `fixed:balanced` | 0.686 | 107382 | 53764 | `balanced` | 0 | 4776.40 | 0.002 | 9.375 |
| `fixed:cross_thread` | 0.688 | 114604 | 54784 | `cross_thread` | 0 | 5211.85 | 0.002 | 9.375 |
| `fixed:deterministic_latency` | 0.723 | 112359 | 55168 | `deterministic_latency` | 0 | 5211.85 | 0.002 | 9.375 |
| `fixed:throughput_cache` | 0.754 | 106585 | 54784 | `throughput_cache` | 0 | 5211.85 | 0.002 | 9.375 |

## `fragmentation_drift:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.150 | 110276 | 47732 | `compact_rss` | 0 | 108.86 | 0.002 | 9.821 |
| `fixed:large_object` | 0.383 | 109067 | 52876 | `large_object` | 0 | 3640.98 | 0.002 | 9.034 |
| `fixed:fragmentation_stable` | 0.570 | 106279 | 53784 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.764 |
| `fixed:deterministic_latency` | 0.684 | 112257 | 56448 | `deterministic_latency` | 0 | 5266.29 | 0.002 | 9.764 |
| `fixed:balanced` | 0.705 | 104408 | 55252 | `balanced` | 0 | 4939.69 | 0.002 | 9.764 |
| `fixed:cross_thread` | 0.719 | 108878 | 56448 | `cross_thread` | 0 | 5266.29 | 0.002 | 9.764 |
| `fixed:throughput_cache` | 0.725 | 108321 | 56448 | `throughput_cache` | 0 | 5266.29 | 0.002 | 9.764 |

## `fragmentation_drift:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.179 | 108915 | 47964 | `compact_rss` | 0 | 108.86 | 0.002 | 9.817 |
| `fixed:large_object` | 0.371 | 115897 | 53180 | `large_object` | 0 | 3630.78 | 0.002 | 8.538 |
| `fixed:fragmentation_stable` | 0.593 | 106382 | 53872 | `fragmentation_stable` | 0 | 3510.86 | 0.002 | 9.772 |
| `fixed:balanced` | 0.691 | 104379 | 54716 | `balanced` | 0 | 4708.36 | 0.002 | 9.772 |
| `fixed:throughput_cache` | 0.725 | 110647 | 56180 | `throughput_cache` | 0 | 5089.38 | 0.002 | 9.772 |
| `fixed:cross_thread` | 0.738 | 108152 | 56064 | `cross_thread` | 0 | 5089.38 | 0.002 | 9.772 |
| `fixed:deterministic_latency` | 0.758 | 106322 | 56192 | `deterministic_latency` | 0 | 5089.38 | 0.002 | 9.772 |

## `fragmentation_drift:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.158 | 122985 | 47260 | `compact_rss` | 0 | 108.86 | 0.002 | 9.764 |
| `fixed:large_object` | 0.400 | 129342 | 53004 | `large_object` | 0 | 3667.35 | 0.002 | 8.568 |
| `fixed:fragmentation_stable` | 0.638 | 111978 | 53816 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.677 |
| `fixed:deterministic_latency` | 0.692 | 123555 | 55296 | `deterministic_latency` | 0 | 4749.18 | 0.002 | 9.677 |
| `fixed:balanced` | 0.707 | 119500 | 55296 | `balanced` | 0 | 4640.32 | 0.002 | 9.677 |
| `fixed:cross_thread` | 0.718 | 119456 | 55424 | `cross_thread` | 0 | 4749.18 | 0.002 | 9.677 |
| `fixed:throughput_cache` | 0.744 | 116538 | 55680 | `throughput_cache` | 0 | 4749.18 | 0.002 | 9.677 |

## `fragmentation_drift:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.212 | 105131 | 47380 | `compact_rss` | 0 | 108.86 | 0.002 | 9.606 |
| `fixed:large_object` | 0.432 | 112796 | 53272 | `large_object` | 0 | 3709.02 | 0.002 | 8.451 |
| `fixed:fragmentation_stable` | 0.614 | 104729 | 53808 | `fragmentation_stable` | 0 | 3592.51 | 0.002 | 9.538 |
| `fixed:balanced` | 0.667 | 120381 | 55612 | `balanced` | 0 | 4898.87 | 0.002 | 9.538 |
| `fixed:throughput_cache` | 0.686 | 123378 | 56192 | `throughput_cache` | 0 | 5062.17 | 0.002 | 9.538 |
| `fixed:deterministic_latency` | 0.703 | 117823 | 56064 | `deterministic_latency` | 0 | 5062.17 | 0.002 | 9.538 |
| `fixed:cross_thread` | 0.746 | 108354 | 56064 | `cross_thread` | 0 | 5062.17 | 0.002 | 9.538 |

## `fragmentation_drift:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.170 | 112984 | 47352 | `compact_rss` | 0 | 108.86 | 0.002 | 9.679 |
| `fixed:large_object` | 0.353 | 126689 | 52384 | `large_object` | 0 | 3667.35 | 0.002 | 8.566 |
| `fixed:fragmentation_stable` | 0.545 | 112994 | 53156 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.608 |
| `fixed:balanced` | 0.696 | 101180 | 54608 | `balanced` | 0 | 4694.75 | 0.002 | 9.608 |
| `fixed:throughput_cache` | 0.707 | 117032 | 55936 | `throughput_cache` | 0 | 4966.91 | 0.002 | 9.608 |
| `fixed:deterministic_latency` | 0.709 | 116504 | 55936 | `deterministic_latency` | 0 | 4966.91 | 0.002 | 9.608 |
| `fixed:cross_thread` | 0.717 | 115813 | 56064 | `cross_thread` | 0 | 4966.91 | 0.002 | 9.608 |

## `fragmentation_drift:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.209 | 113329 | 46844 | `compact_rss` | 0 | 108.86 | 0.002 | 9.504 |
| `fixed:large_object` | 0.365 | 126686 | 52020 | `large_object` | 0 | 3681.81 | 0.002 | 8.674 |
| `fixed:fragmentation_stable` | 0.561 | 121807 | 53156 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.455 |
| `fixed:cross_thread` | 0.697 | 124691 | 55424 | `cross_thread` | 0 | 4926.09 | 0.002 | 9.455 |
| `fixed:deterministic_latency` | 0.708 | 122739 | 55424 | `deterministic_latency` | 0 | 4926.09 | 0.002 | 9.455 |
| `fixed:balanced` | 0.732 | 112459 | 54700 | `balanced` | 0 | 4762.79 | 0.002 | 9.455 |
| `fixed:throughput_cache` | 0.739 | 116470 | 55296 | `throughput_cache` | 0 | 4926.09 | 0.002 | 9.455 |

## `fragmentation_drift:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.191 | 114846 | 47764 | `compact_rss` | 0 | 108.86 | 0.002 | 9.675 |
| `fixed:large_object` | 0.375 | 122108 | 53276 | `large_object` | 0 | 3681.81 | 0.002 | 8.626 |
| `fixed:fragmentation_stable` | 0.565 | 115455 | 53604 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.623 |
| `fixed:balanced` | 0.687 | 112320 | 54700 | `balanced` | 0 | 4817.22 | 0.002 | 9.623 |
| `fixed:cross_thread` | 0.709 | 118086 | 56064 | `cross_thread` | 0 | 5143.81 | 0.002 | 9.623 |
| `fixed:throughput_cache` | 0.723 | 117162 | 56192 | `throughput_cache` | 0 | 5143.81 | 0.002 | 9.623 |
| `fixed:deterministic_latency` | 0.737 | 116203 | 56320 | `deterministic_latency` | 0 | 5143.81 | 0.002 | 9.623 |

## `fragmentation_drift:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.203 | 112093 | 48176 | `compact_rss` | 0 | 108.86 | 0.002 | 9.574 |
| `fixed:large_object` | 0.362 | 124370 | 52972 | `large_object` | 0 | 3669.05 | 0.002 | 8.432 |
| `fixed:fragmentation_stable` | 0.541 | 117340 | 53436 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.491 |
| `fixed:balanced` | 0.654 | 117081 | 54752 | `balanced` | 0 | 4858.05 | 0.002 | 9.491 |
| `fixed:cross_thread` | 0.728 | 116670 | 55936 | `cross_thread` | 0 | 5239.07 | 0.002 | 9.491 |
| `fixed:throughput_cache` | 0.729 | 115555 | 55808 | `throughput_cache` | 0 | 5239.07 | 0.002 | 9.491 |
| `fixed:deterministic_latency` | 0.769 | 110326 | 55936 | `deterministic_latency` | 0 | 5239.07 | 0.002 | 9.491 |

## `fragmentation_drift:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.208 | 111073 | 47560 | `compact_rss` | 0 | 108.86 | 0.002 | 9.477 |
| `fixed:large_object` | 0.378 | 117497 | 52804 | `large_object` | 0 | 3668.20 | 0.002 | 8.506 |
| `fixed:fragmentation_stable` | 0.568 | 113651 | 53520 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.406 |
| `fixed:balanced` | 0.656 | 116512 | 55128 | `balanced` | 0 | 4803.61 | 0.002 | 9.406 |
| `fixed:cross_thread` | 0.684 | 118360 | 55936 | `cross_thread` | 0 | 5075.77 | 0.002 | 9.406 |
| `fixed:throughput_cache` | 0.732 | 113818 | 55936 | `throughput_cache` | 0 | 5075.77 | 0.002 | 9.406 |
| `fixed:deterministic_latency` | 0.763 | 110546 | 55808 | `deterministic_latency` | 0 | 5075.77 | 0.002 | 9.406 |

## `large_burst:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.087 | 335379 | 14208 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.087 | 324793 | 14208 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.088 | 297904 | 14208 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.088 | 297037 | 14208 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.089 | 288290 | 14208 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 645788 | 14208 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 51506 | 14208 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.086 | 326010 | 14592 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.088 | 300739 | 14592 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.089 | 278197 | 14592 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.473 | 314236 | 14720 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.473 | 298461 | 14720 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.564 | 635145 | 14720 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.600 | 48863 | 14720 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.084 | 318443 | 14976 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.084 | 308264 | 14976 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.468 | 340824 | 15104 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.468 | 336031 | 15104 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.469 | 332040 | 15104 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.564 | 415877 | 15104 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.600 | 54708 | 15104 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.086 | 338679 | 15488 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.087 | 315560 | 15488 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.088 | 298419 | 15488 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.088 | 295713 | 15488 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.090 | 259964 | 15488 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 588136 | 15488 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 52785 | 15488 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.084 | 324070 | 15744 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.085 | 314231 | 15744 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.086 | 294293 | 15744 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.086 | 285833 | 15744 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.087 | 268302 | 15744 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 475367 | 15744 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 48983 | 15744 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.089 | 273597 | 16128 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.472 | 325967 | 16256 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.472 | 325812 | 16256 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.473 | 298519 | 16256 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.473 | 288987 | 16256 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.564 | 580463 | 16256 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.600 | 50359 | 16256 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.087 | 403038 | 16512 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.087 | 400033 | 16512 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.088 | 367064 | 16512 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.089 | 335882 | 16512 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.214 | 62620 | 16512 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |
| `fixed:cross_thread` | 0.472 | 396841 | 16640 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.564 | 768049 | 16640 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |

## `large_burst:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.085 | 431062 | 16896 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.085 | 428992 | 16896 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.085 | 427644 | 16896 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.086 | 395417 | 16896 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.087 | 368461 | 16896 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 678541 | 16896 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 64342 | 16896 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.085 | 415423 | 17280 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.085 | 408958 | 17280 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.086 | 389469 | 17280 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.086 | 375426 | 17280 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.088 | 339999 | 17280 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 650260 | 17280 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 62621 | 17280 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.086 | 337713 | 17536 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.086 | 330392 | 17536 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.470 | 372886 | 17664 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.472 | 343910 | 17664 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.472 | 335559 | 17664 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.564 | 634372 | 17664 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.600 | 50468 | 17664 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.085 | 365814 | 17920 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.087 | 320609 | 17920 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.087 | 317235 | 17920 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.087 | 315404 | 17920 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.088 | 305961 | 17920 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 630793 | 17920 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 52211 | 17920 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.086 | 299062 | 18304 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.086 | 298280 | 18304 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.086 | 295800 | 18304 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.087 | 290727 | 18304 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.214 | 48166 | 18304 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |
| `fixed:cross_thread` | 0.472 | 285707 | 18432 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.564 | 518511 | 18432 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |

## `large_burst:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.085 | 327917 | 18816 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.086 | 310330 | 18816 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.087 | 299076 | 18816 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.087 | 298159 | 18816 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.088 | 266924 | 18816 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 577744 | 18816 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 46718 | 18816 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.086 | 362564 | 19200 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.086 | 344826 | 19200 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.086 | 343977 | 19200 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.087 | 326672 | 19200 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.088 | 312667 | 19200 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 619770 | 19200 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 54525 | 19200 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.086 | 369586 | 19584 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.087 | 354117 | 19584 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.087 | 340325 | 19584 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.088 | 326783 | 19584 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.088 | 321362 | 19584 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 681226 | 19584 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 54244 | 19584 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.087 | 349230 | 19968 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.087 | 344039 | 19968 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.087 | 339110 | 19968 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.087 | 336402 | 19968 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.089 | 293478 | 19968 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.214 | 53012 | 19968 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |
| `fixed:large_object` | 0.564 | 693407 | 20096 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |

## `large_burst:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.085 | 306184 | 20480 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.086 | 290060 | 20480 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.087 | 281299 | 20480 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.087 | 281221 | 20480 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.089 | 249392 | 20480 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 515854 | 20480 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 46603 | 20480 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.087 | 331884 | 20864 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.087 | 318608 | 20864 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.088 | 312621 | 20864 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.088 | 310231 | 20864 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.088 | 297627 | 20864 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 645985 | 20864 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 52313 | 20864 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.087 | 329785 | 21248 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.088 | 295922 | 21248 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.089 | 288710 | 21248 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.089 | 280314 | 21248 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.091 | 253883 | 21248 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 664620 | 21248 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 49638 | 21248 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.086 | 345016 | 21632 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.087 | 316683 | 21632 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.088 | 301265 | 21632 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.088 | 297552 | 21632 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.089 | 284668 | 21632 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.178 | 625225 | 21632 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `fixed:compact_rss` | 0.214 | 49941 | 21632 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `latency_loop:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.085 | 4874677 | 14336 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 8396732 | 14336 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.190 | 7618006 | 14336 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.195 | 7352676 | 14336 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.196 | 7294847 | 14336 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.198 | 7174144 | 14336 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.251 | 5195338 | 14336 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.077 | 6718972 | 14848 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 7832490 | 14848 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.204 | 7419888 | 14848 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.224 | 7132688 | 14848 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.229 | 7059987 | 14848 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.247 | 6830241 | 14848 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.263 | 6630558 | 14848 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.085 | 5210125 | 15232 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 7770423 | 15232 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7767794 | 15232 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.181 | 7673811 | 15232 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.185 | 7475691 | 15232 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.235 | 5846322 | 15232 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.237 | 5795001 | 15232 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.036 | 6734007 | 15616 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 8067511 | 15616 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.179 | 8033795 | 15616 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.183 | 7865525 | 15616 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.184 | 7806985 | 15616 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 7218587 | 15616 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.263 | 5484987 | 15616 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.204 | 7366407 | 15872 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.263 | 6909572 | 15872 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.448 | 7078869 | 16000 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.564 | 7585239 | 16000 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.597 | 7308376 | 16000 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.605 | 7240688 | 16000 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.619 | 7129280 | 16000 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.016 | 6985771 | 16256 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 7642416 | 16256 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.186 | 7308096 | 16256 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 6795423 | 16256 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.212 | 6412195 | 16256 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.243 | 5573328 | 16256 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.263 | 5132358 | 16256 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.016 | 8582092 | 16768 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 9813834 | 16768 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.192 | 8755329 | 16768 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.195 | 8591421 | 16768 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.197 | 8415089 | 16768 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.225 | 6947577 | 16768 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.263 | 5619344 | 16768 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.085 | 6758812 | 17024 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 9113899 | 17024 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.188 | 8754076 | 17024 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.192 | 8639309 | 17024 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.206 | 8195378 | 17024 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.214 | 7938239 | 17024 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.240 | 7272860 | 17024 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.030 | 8306308 | 17408 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 9789425 | 17408 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.191 | 9097921 | 17408 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.195 | 8901747 | 17408 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.199 | 8725004 | 17408 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.237 | 7241107 | 17408 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.263 | 6473373 | 17408 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.078 | 7107068 | 17792 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 8087996 | 17792 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.190 | 7923458 | 17792 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.201 | 7779678 | 17792 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.205 | 7727377 | 17792 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.226 | 7455454 | 17792 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.263 | 7030124 | 17792 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.178 | 7757364 | 18048 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.232 | 6322494 | 18048 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.433 | 6469411 | 18176 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.576 | 7393181 | 18176 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.577 | 7344235 | 18176 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.588 | 7030471 | 18176 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.649 | 5708756 | 18176 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.041 | 6922275 | 18560 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 7442598 | 18560 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.208 | 7057385 | 18560 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.220 | 6907466 | 18560 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.239 | 6688582 | 18560 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.239 | 6688690 | 18560 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.263 | 6436998 | 18560 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.010 | 6562723 | 18944 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7353084 | 18944 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.180 | 7227960 | 18944 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.182 | 7085697 | 18944 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.204 | 5673752 | 18944 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.211 | 5333912 | 18944 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.263 | 3706049 | 18944 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.041 | 7662627 | 19328 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 8157397 | 19328 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.183 | 8091446 | 19328 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.210 | 7772636 | 19328 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.211 | 7762405 | 19328 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.257 | 7254107 | 19328 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.263 | 7196563 | 19328 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.178 | 7872776 | 19712 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 7866891 | 19712 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.263 | 4819393 | 19712 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.405 | 6895957 | 19840 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.575 | 7285974 | 19840 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.595 | 6380195 | 19840 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.637 | 5097100 | 19840 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.016 | 7178095 | 20224 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 8014760 | 20224 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.180 | 7921929 | 20224 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.193 | 7236784 | 20224 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.196 | 7138989 | 20224 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.214 | 6389580 | 20224 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.263 | 4992775 | 20224 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.051 | 7155844 | 20608 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 8411757 | 20608 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.198 | 7889593 | 20608 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.207 | 7652494 | 20608 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.214 | 7488596 | 20608 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.230 | 7137600 | 20608 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.263 | 6509694 | 20608 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.085 | 5108817 | 20992 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7894841 | 20992 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.180 | 7803660 | 20992 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.192 | 7242046 | 20992 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 6921731 | 20992 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.208 | 6627673 | 20992 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.221 | 6205784 | 20992 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.005 | 7600894 | 21376 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7706388 | 21376 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.192 | 7423997 | 21376 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.213 | 7016816 | 21376 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.217 | 6950467 | 21376 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.217 | 6949648 | 21376 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.263 | 6222056 | 21376 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.085 | 5234530 | 21760 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.216 | 6255027 | 21760 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.230 | 5907208 | 21760 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.235 | 5795086 | 21760 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.564 | 7435680 | 21888 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.567 | 7328592 | 21888 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.600 | 6309894 | 21888 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `remote_queue:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.048 | 564475 | 45824 | `large_object` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.049 | 556250 | 45824 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.074 | 565312 | 45952 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.142 | 557735 | 46208 | `deterministic_latency` | 0 | 2224.24 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.214 | 148963 | 45696 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:cross_thread` | 0.387 | 636712 | 47744 | `cross_thread` | 0 | 2795.49 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 681374 | 48256 | `throughput_cache` | 0 | 6453.94 | 0.186 | 0.025 |

## `remote_queue:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.025 | 620013 | 46080 | `large_object` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.027 | 576121 | 46080 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.054 | 590429 | 46208 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.101 | 609420 | 46336 | `deterministic_latency` | 0 | 2139.16 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.214 | 152938 | 46080 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:cross_thread` | 0.333 | 639240 | 47616 | `cross_thread` | 0 | 2783.34 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 647097 | 48384 | `throughput_cache` | 0 | 6478.24 | 0.186 | 0.026 |

## `remote_queue:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.055 | 556117 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.187 | 0.022 |
| `fixed:large_object` | 0.055 | 555632 | 46336 | `large_object` | 0 | 972.34 | 0.187 | 0.022 |
| `fixed:balanced` | 0.059 | 633550 | 46336 | `balanced` | 0 | 1227.59 | 0.187 | 0.022 |
| `fixed:deterministic_latency` | 0.087 | 609953 | 46336 | `deterministic_latency` | 0 | 2212.08 | 0.187 | 0.022 |
| `fixed:compact_rss` | 0.214 | 158224 | 46208 | `compact_rss` | 0 | 109.39 | 0.187 | 0.272 |
| `fixed:cross_thread` | 0.375 | 670382 | 47872 | `cross_thread` | 0 | 2807.64 | 0.187 | 0.025 |
| `fixed:throughput_cache` | 0.566 | 774748 | 48384 | `throughput_cache` | 0 | 6478.24 | 0.187 | 0.026 |

## `remote_queue:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.037 | 581523 | 46336 | `balanced` | 0 | 1239.74 | 0.183 | 0.022 |
| `fixed:large_object` | 0.049 | 586763 | 46464 | `large_object` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:fragmentation_stable` | 0.053 | 526676 | 46464 | `fragmentation_stable` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.081 | 654514 | 46464 | `deterministic_latency` | 0 | 2212.08 | 0.183 | 0.022 |
| `fixed:compact_rss` | 0.235 | 156698 | 46464 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |
| `fixed:cross_thread` | 0.365 | 614936 | 48128 | `cross_thread` | 0 | 2795.49 | 0.183 | 0.025 |
| `fixed:throughput_cache` | 0.566 | 696300 | 48768 | `throughput_cache` | 0 | 6417.47 | 0.183 | 0.026 |

## `remote_queue:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.035 | 553695 | 45696 | `balanced` | 0 | 1239.74 | 0.186 | 0.022 |
| `fixed:large_object` | 0.054 | 552892 | 45824 | `large_object` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.084 | 472219 | 45952 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.112 | 596562 | 45952 | `deterministic_latency` | 0 | 2187.77 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.266 | 130930 | 45952 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:cross_thread` | 0.362 | 557582 | 47104 | `cross_thread` | 0 | 2783.34 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 651390 | 47616 | `throughput_cache` | 0 | 6466.09 | 0.186 | 0.025 |

## `remote_queue:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.035 | 572951 | 45696 | `balanced` | 0 | 1227.59 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.054 | 521214 | 45824 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:large_object` | 0.056 | 502588 | 45824 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.111 | 567895 | 45952 | `deterministic_latency` | 0 | 2224.24 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.263 | 144194 | 45952 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:cross_thread` | 0.394 | 529819 | 47360 | `cross_thread` | 0 | 2759.03 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 654728 | 47744 | `throughput_cache` | 0 | 6466.09 | 0.185 | 0.025 |

## `remote_queue:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.032 | 635909 | 46080 | `large_object` | 0 | 972.34 | 0.182 | 0.022 |
| `fixed:fragmentation_stable` | 0.052 | 676757 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.182 | 0.022 |
| `fixed:balanced` | 0.058 | 688202 | 46208 | `balanced` | 0 | 1203.28 | 0.182 | 0.022 |
| `fixed:deterministic_latency` | 0.105 | 745779 | 46336 | `deterministic_latency` | 0 | 2199.93 | 0.182 | 0.022 |
| `fixed:compact_rss` | 0.236 | 182055 | 46208 | `compact_rss` | 0 | 109.39 | 0.182 | 0.272 |
| `fixed:cross_thread` | 0.359 | 723120 | 47744 | `cross_thread` | 0 | 2795.49 | 0.182 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 864359 | 48384 | `throughput_cache` | 0 | 6478.24 | 0.182 | 0.026 |

## `remote_queue:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.028 | 691110 | 46208 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.029 | 669430 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.042 | 549420 | 46208 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.080 | 678330 | 46336 | `deterministic_latency` | 0 | 2114.85 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.234 | 179280 | 46336 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:cross_thread` | 0.347 | 729221 | 48000 | `cross_thread` | 0 | 2722.56 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 802242 | 48768 | `throughput_cache` | 0 | 6441.78 | 0.185 | 0.026 |

## `remote_queue:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.029 | 696516 | 46080 | `large_object` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.056 | 628516 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.062 | 654717 | 46208 | `balanced` | 0 | 1239.74 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.111 | 685475 | 46336 | `deterministic_latency` | 0 | 2175.62 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.214 | 181795 | 46080 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:cross_thread` | 0.418 | 720312 | 47872 | `cross_thread` | 0 | 2807.64 | 0.186 | 0.025 |
| `fixed:throughput_cache` | 0.566 | 829565 | 48128 | `throughput_cache` | 0 | 6478.24 | 0.186 | 0.025 |

## `remote_queue:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.034 | 733389 | 46336 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:large_object` | 0.034 | 589539 | 46336 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.078 | 554555 | 46592 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.084 | 746768 | 46464 | `deterministic_latency` | 0 | 2248.55 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.236 | 173685 | 46464 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:cross_thread` | 0.406 | 618633 | 48256 | `cross_thread` | 0 | 2783.34 | 0.185 | 0.025 |
| `fixed:throughput_cache` | 0.566 | 844973 | 48640 | `throughput_cache` | 0 | 6490.40 | 0.185 | 0.025 |

## `remote_queue:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.030 | 590008 | 45952 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.036 | 586479 | 45952 | `balanced` | 0 | 1166.81 | 0.186 | 0.022 |
| `fixed:large_object` | 0.048 | 595673 | 46080 | `large_object` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.079 | 662021 | 46080 | `deterministic_latency` | 0 | 2163.47 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.214 | 162453 | 45952 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:cross_thread` | 0.333 | 672363 | 47744 | `cross_thread` | 0 | 2686.10 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 744032 | 48640 | `throughput_cache` | 0 | 6490.40 | 0.186 | 0.026 |

## `remote_queue:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.032 | 526778 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:large_object` | 0.054 | 514149 | 46336 | `large_object` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:balanced` | 0.056 | 598427 | 46336 | `balanced` | 0 | 1203.28 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.106 | 601788 | 46464 | `deterministic_latency` | 0 | 2199.93 | 0.183 | 0.022 |
| `fixed:compact_rss` | 0.236 | 142685 | 46336 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |
| `fixed:cross_thread` | 0.314 | 553629 | 47616 | `cross_thread` | 0 | 2637.48 | 0.183 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 723942 | 48512 | `throughput_cache` | 0 | 6466.09 | 0.183 | 0.026 |

## `remote_queue:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.032 | 486454 | 46464 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:large_object` | 0.054 | 511537 | 46592 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.085 | 508264 | 46720 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.109 | 555288 | 46720 | `deterministic_latency` | 0 | 2151.31 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.214 | 138869 | 46464 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:cross_thread` | 0.345 | 544221 | 47872 | `cross_thread` | 0 | 2746.87 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 634897 | 48512 | `throughput_cache` | 0 | 6417.47 | 0.185 | 0.026 |

## `remote_queue:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.036 | 564813 | 46592 | `balanced` | 0 | 1191.12 | 0.184 | 0.022 |
| `fixed:large_object` | 0.053 | 548435 | 46720 | `large_object` | 0 | 972.34 | 0.184 | 0.022 |
| `fixed:fragmentation_stable` | 0.054 | 518740 | 46720 | `fragmentation_stable` | 0 | 972.34 | 0.184 | 0.022 |
| `fixed:deterministic_latency` | 0.108 | 567368 | 46848 | `deterministic_latency` | 0 | 2212.08 | 0.184 | 0.023 |
| `fixed:compact_rss` | 0.236 | 154004 | 46720 | `compact_rss` | 0 | 109.39 | 0.184 | 0.272 |
| `fixed:cross_thread` | 0.336 | 629979 | 48128 | `cross_thread` | 0 | 2771.18 | 0.184 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 710594 | 48896 | `throughput_cache` | 0 | 6453.94 | 0.184 | 0.026 |

## `remote_queue:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.036 | 578844 | 46208 | `balanced` | 0 | 1178.97 | 0.185 | 0.022 |
| `fixed:large_object` | 0.050 | 572806 | 46336 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.051 | 547515 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.140 | 635473 | 46720 | `deterministic_latency` | 0 | 2199.93 | 0.185 | 0.023 |
| `fixed:compact_rss` | 0.253 | 152033 | 46464 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:cross_thread` | 0.369 | 639211 | 48128 | `cross_thread` | 0 | 2783.34 | 0.185 | 0.025 |
| `fixed:throughput_cache` | 0.566 | 739082 | 48768 | `throughput_cache` | 0 | 6453.94 | 0.185 | 0.026 |

## `remote_queue:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.036 | 525723 | 46208 | `balanced` | 0 | 1191.12 | 0.183 | 0.022 |
| `fixed:large_object` | 0.046 | 582613 | 46336 | `large_object` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:fragmentation_stable` | 0.048 | 543221 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.118 | 550667 | 46592 | `deterministic_latency` | 0 | 2102.69 | 0.183 | 0.022 |
| `fixed:compact_rss` | 0.234 | 150577 | 46336 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |
| `fixed:cross_thread` | 0.307 | 580964 | 47744 | `cross_thread` | 0 | 2649.64 | 0.183 | 0.025 |
| `fixed:throughput_cache` | 0.566 | 645125 | 48768 | `throughput_cache` | 0 | 6453.94 | 0.183 | 0.026 |

## `remote_queue:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.068 | 527666 | 45824 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.069 | 516254 | 45824 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.071 | 573242 | 45824 | `balanced` | 0 | 1178.97 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.114 | 603049 | 45952 | `deterministic_latency` | 0 | 2090.54 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.214 | 145678 | 45568 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:cross_thread` | 0.338 | 556793 | 47360 | `cross_thread` | 0 | 2759.03 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 694495 | 48256 | `throughput_cache` | 0 | 6478.24 | 0.185 | 0.026 |

## `remote_queue:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.032 | 519366 | 45952 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.038 | 534691 | 45952 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:large_object` | 0.048 | 578218 | 46080 | `large_object` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.133 | 616315 | 46464 | `deterministic_latency` | 0 | 2102.69 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.251 | 149282 | 46208 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:cross_thread` | 0.318 | 619252 | 47616 | `cross_thread` | 0 | 2771.18 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 708022 | 48640 | `throughput_cache` | 0 | 6466.09 | 0.186 | 0.026 |

## `remote_queue:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.030 | 501694 | 45952 | `large_object` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.034 | 521623 | 45952 | `balanced` | 0 | 1166.81 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.051 | 440936 | 46080 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.096 | 512428 | 46208 | `deterministic_latency` | 0 | 2114.85 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.214 | 129875 | 45952 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:cross_thread` | 0.343 | 535391 | 47872 | `cross_thread` | 0 | 2783.34 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.566 | 632048 | 48768 | `throughput_cache` | 0 | 6466.09 | 0.186 | 0.026 |

## `remote_queue:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.034 | 567248 | 46208 | `balanced` | 0 | 1203.28 | 0.184 | 0.022 |
| `fixed:large_object` | 0.046 | 561563 | 46336 | `large_object` | 0 | 972.34 | 0.184 | 0.022 |
| `fixed:fragmentation_stable` | 0.047 | 542947 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.184 | 0.022 |
| `fixed:deterministic_latency` | 0.078 | 599090 | 46336 | `deterministic_latency` | 0 | 2163.47 | 0.184 | 0.022 |
| `fixed:compact_rss` | 0.214 | 150064 | 46208 | `compact_rss` | 0 | 109.39 | 0.184 | 0.272 |
| `fixed:cross_thread` | 0.318 | 594409 | 47872 | `cross_thread` | 0 | 2795.49 | 0.184 | 0.025 |
| `fixed:throughput_cache` | 0.566 | 646071 | 48896 | `throughput_cache` | 0 | 6466.09 | 0.184 | 0.026 |

## `rss_peak_release:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.117 | 1663667 | 14208 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1850918 | 14208 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.179 | 1736544 | 14208 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 1504211 | 14208 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.179 | 1404392 | 14208 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 46702 | 14208 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.502 | 1715528 | 14336 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |

## `rss_peak_release:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.116 | 1747810 | 14720 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.117 | 1282152 | 14720 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1750078 | 14720 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 1873190 | 14720 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1829807 | 14720 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.178 | 1773407 | 14720 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 48908 | 14720 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.117 | 1475717 | 15104 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.118 | 932417 | 15104 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1721464 | 15104 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1783141 | 15104 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 1506475 | 15104 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.180 | 891393 | 15104 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 42996 | 15104 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.117 | 1712331 | 15488 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.117 | 1641959 | 15488 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1853615 | 15488 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 1951311 | 15488 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1936266 | 15488 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.178 | 1859532 | 15488 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 49142 | 15488 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.177 | 1396561 | 15744 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.502 | 1390314 | 15872 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.505 | 642986 | 15872 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.564 | 1573356 | 15872 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.564 | 1366253 | 15872 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.565 | 1140751 | 15872 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.600 | 38794 | 15872 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.117 | 1524707 | 16256 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.118 | 1118865 | 16256 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1558877 | 16256 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 1755109 | 16256 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 1507791 | 16256 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.179 | 1487995 | 16256 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 42493 | 16256 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.116 | 2087840 | 16640 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.117 | 1987776 | 16640 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 2158676 | 16640 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 2229305 | 16640 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 2143339 | 16640 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 2088996 | 16640 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 56356 | 16640 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.177 | 2171633 | 16896 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 2194084 | 16896 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.179 | 1637748 | 16896 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 58121 | 16896 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:fragmentation_stable` | 0.502 | 2081590 | 17024 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.502 | 2049264 | 17024 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.564 | 1915019 | 17024 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |

## `rss_peak_release:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.117 | 1985912 | 17280 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.117 | 1776672 | 17280 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 2168314 | 17280 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 2314687 | 17280 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 2022551 | 17280 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.179 | 1865982 | 17280 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 54822 | 17280 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.117 | 1655628 | 17664 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.118 | 1257805 | 17664 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1940005 | 17664 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 1964746 | 17664 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.178 | 1931382 | 17664 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1857953 | 17664 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 50798 | 17664 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.177 | 1445435 | 17920 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:large_object` | 0.502 | 1531605 | 18048 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.503 | 1336837 | 18048 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.564 | 1750029 | 18048 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.564 | 1717423 | 18048 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.564 | 1547430 | 18048 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.600 | 43278 | 18048 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.117 | 1631944 | 18432 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.117 | 1331013 | 18432 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1568955 | 18432 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1822846 | 18432 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 1663686 | 18432 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.179 | 1301338 | 18432 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 44254 | 18432 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.117 | 1615512 | 18816 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.117 | 1369129 | 18816 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1300458 | 18816 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1771602 | 18816 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 1519865 | 18816 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.179 | 1451351 | 18816 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 41989 | 18816 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.117 | 1740209 | 19200 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1970370 | 19200 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.179 | 1828753 | 19200 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 1776544 | 19200 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.179 | 1432491 | 19200 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 49045 | 19200 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.502 | 1710724 | 19328 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |

## `rss_peak_release:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.117 | 1758801 | 19584 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1691878 | 19584 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1968085 | 19584 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.179 | 1777185 | 19584 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.179 | 1686661 | 19584 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 45959 | 19584 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.502 | 1552151 | 19712 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |

## `rss_peak_release:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.116 | 1848325 | 20096 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.117 | 1718039 | 20096 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1512818 | 20096 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 1932882 | 20096 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.178 | 1907305 | 20096 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.179 | 1800227 | 20096 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 48311 | 20096 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.117 | 1444556 | 20480 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.117 | 1202030 | 20480 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1462798 | 20480 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.178 | 1733950 | 20480 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1690869 | 20480 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.179 | 1304517 | 20480 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 43143 | 20480 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.177 | 1765835 | 20864 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.502 | 1723130 | 20992 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.502 | 1553652 | 20992 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.564 | 1971512 | 20992 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.564 | 1763529 | 20992 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.564 | 1641468 | 20992 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.600 | 46584 | 20992 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.117 | 1519972 | 21248 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.117 | 1464510 | 21248 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.177 | 1588579 | 21248 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.178 | 1851727 | 21248 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.178 | 1837742 | 21248 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.179 | 1506995 | 21248 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 44723 | 21248 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.177 | 1590341 | 21632 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.178 | 1759503 | 21632 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:large_object` | 0.502 | 1644270 | 21760 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.502 | 1617782 | 21760 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.564 | 1696964 | 21760 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.564 | 1598881 | 21760 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.600 | 46988 | 21760 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `throughput_churn:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.179 | 4894193 | 14080 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.564 | 6723818 | 14208 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.564 | 6374516 | 14208 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.564 | 5800214 | 14208 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.564 | 4832439 | 14208 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.564 | 4740305 | 14208 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.600 | 45142 | 14208 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.178 | 7338899 | 14592 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7205477 | 14592 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.178 | 6959546 | 14592 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 6771837 | 14592 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 6759668 | 14592 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.179 | 6036960 | 14592 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 55386 | 14592 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.178 | 7482615 | 14848 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7374494 | 14848 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 7278671 | 14848 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.179 | 4516146 | 14848 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 57116 | 14848 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |
| `fixed:large_object` | 0.564 | 7385977 | 14976 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.564 | 5720026 | 14976 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |

## `throughput_churn:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.178 | 7580100 | 15360 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 7544714 | 15360 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 6694747 | 15360 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 6457757 | 15360 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 6356722 | 15360 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.179 | 5358494 | 15360 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 56329 | 15360 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.178 | 7066014 | 15744 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 6569331 | 15744 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.178 | 6471339 | 15744 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 6353668 | 15744 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 6335036 | 15744 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 6136805 | 15744 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 50060 | 15744 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.178 | 7329547 | 16128 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7290763 | 16128 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 7095836 | 16128 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 6389879 | 16128 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.178 | 6043463 | 16128 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.179 | 5708688 | 16128 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 54046 | 16128 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.178 | 8413949 | 16512 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 8361180 | 16512 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 8327216 | 16512 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.178 | 7796877 | 16512 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 7550482 | 16512 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 7366324 | 16512 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 65894 | 16512 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.178 | 7829322 | 16768 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 6858766 | 16768 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.564 | 8142040 | 16896 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.564 | 6985657 | 16896 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.564 | 6530612 | 16896 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.564 | 6046100 | 16896 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.600 | 60328 | 16896 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.178 | 8091446 | 17024 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 8081556 | 17024 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 7779009 | 17024 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.179 | 6426184 | 17024 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.179 | 6204340 | 17024 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.179 | 5439857 | 17024 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 64284 | 17024 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.178 | 8463876 | 17536 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.178 | 8445202 | 17536 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 8405994 | 17536 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 8401302 | 17536 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 8133980 | 17536 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7932008 | 17536 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 63994 | 17536 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.178 | 8162859 | 17792 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 7985656 | 17792 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7918757 | 17792 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 7845629 | 17792 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 7310630 | 17792 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 7009412 | 17792 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 62396 | 17792 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.178 | 7261411 | 18304 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 7259565 | 18304 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 7179351 | 18304 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.178 | 6918580 | 18304 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 6614826 | 18304 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 6488921 | 18304 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 54095 | 18304 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.178 | 6190553 | 18560 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.179 | 4814234 | 18560 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.564 | 6919261 | 18688 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.564 | 6683066 | 18688 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.564 | 6623091 | 18688 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.564 | 5868727 | 18688 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.600 | 47732 | 18688 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.179 | 4563240 | 18944 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.564 | 7272364 | 19072 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.564 | 6914553 | 19072 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.564 | 6807332 | 19072 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.564 | 6761029 | 19072 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.564 | 5522725 | 19072 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.600 | 51505 | 19072 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.178 | 7215126 | 19456 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 7150219 | 19456 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 6711119 | 19456 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 6307109 | 19456 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 6271033 | 19456 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.179 | 5950521 | 19456 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 55745 | 19456 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.178 | 6736293 | 19840 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 6656857 | 19840 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.179 | 5436198 | 19840 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 47474 | 19840 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |
| `fixed:cross_thread` | 0.564 | 6527504 | 19968 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.564 | 6318799 | 19968 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.564 | 5006064 | 19968 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |

## `throughput_churn:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.178 | 7482230 | 20224 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 7233544 | 20224 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 7040054 | 20224 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.179 | 5792771 | 20224 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 55689 | 20224 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |
| `fixed:cross_thread` | 0.564 | 7373996 | 20352 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.564 | 6247958 | 20352 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |

## `throughput_churn:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.178 | 6949032 | 20736 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.178 | 6842706 | 20736 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 6583255 | 20736 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 6563310 | 20736 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 6450088 | 20736 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.179 | 5361209 | 20736 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 53246 | 20736 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.178 | 6199147 | 20992 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.564 | 7162681 | 21120 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.564 | 6874270 | 21120 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.564 | 6078104 | 21120 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.564 | 5807201 | 21120 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.564 | 4529344 | 21120 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.600 | 43855 | 21120 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.178 | 6909776 | 21504 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.178 | 6746158 | 21504 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 6628388 | 21504 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.178 | 5765782 | 21504 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.178 | 5707391 | 21504 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.179 | 5625703 | 21504 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 51184 | 21504 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

