# Adaptive Selector Model Evaluation Results

Generated at: `2026-05-08 17:51:47 +0800`

These measurements compare fixed adaptive modes against the measured-cost `model` selector on the generated workload suite. Lower score is better.

## Reproduction Command

```bash
python3 tools/evaluate_selector_model.py --bench-runner ./build/bench_runner --iters 20000 --slots 512 --threads 2 --seeds 10001 10002 10003 10004 10005 10006 10007 10008 10009 10010 10011 10012 10013 10014 10015 10016 10017 10018 10019 10020 --mode-window 64 --mode-cooldown 0 --json-out models/selector_training_dataset.json --markdown-out docs/adaptive_selector_training_dataset.md
```

## Score Definition

Score is normalized per workload across all compared cases:

| Metric | Weight | Direction |
|---|---:|---|
| `ms` | 0.35 | lower is better |
| `peak_rss_kb` | 0.20 | lower is better |
| `mapped_live_ratio` | 0.20 | lower is better |
| `fragmentation_estimate` | 0.10 | lower is better |
| `slow_path_ratio` | 0.10 | lower is better |
| `mode_switches` | 0.05 | lower is better |

## Overall Ranking

| Rank | Case | Mean score | Mean ops/sec | Mean peak RSS KB | Mean switches | Best workload count |
|---:|---|---:|---:|---:|---:|---:|
| 1 | `fixed:large_object` | 0.167 | 2381198 | 29154 | 0.0 | 99 |
| 2 | `fixed:balanced` | 0.310 | 2407222 | 31042 | 0.0 | 20 |
| 3 | `fixed:fragmentation_stable` | 0.314 | 2400718 | 30784 | 0.0 | 6 |
| 4 | `fixed:compact_rss` | 0.413 | 1187670 | 28989 | 0.0 | 15 |
| 5 | `fixed:throughput_cache` | 0.416 | 400461 | 31277 | 0.0 | 0 |
| 6 | `fixed:cross_thread` | 0.420 | 400500 | 31069 | 0.0 | 0 |
| 7 | `fixed:deterministic_latency` | 0.421 | 403488 | 31280 | 0.0 | 0 |

## Per-Workload Winners

| Workload | Best overall | Best fixed mode | Best model selector |
|---|---|---|---|
| `adaptive_mix:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10002` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10003` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10004` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10005` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10006` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10007` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10008` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10009` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10010` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10011` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10012` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10014` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10016` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10017` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10018` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10020` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10002` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10003` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10004` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10005` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10006` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10007` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10008` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10009` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10010` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10011` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10012` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10013` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10014` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10016` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10017` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10018` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10019` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10020` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `large_burst:seed10001` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10002` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10003` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10004` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10005` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10006` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10007` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10008` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10009` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10010` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10011` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10012` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10014` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10016` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10017` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10018` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10020` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10002` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10003` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10004` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10005` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10006` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10007` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10008` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10009` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10010` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10011` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10012` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10014` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10016` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10017` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10018` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10020` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10002` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10003` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10004` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10005` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10006` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10007` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10008` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10009` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10010` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10011` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10012` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10014` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10016` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10017` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10018` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10020` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10002` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10003` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10004` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10005` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10006` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10007` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10008` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10009` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10010` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10011` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10012` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10014` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10016` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10017` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10018` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `rss_peak_release:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `rss_peak_release:seed10020` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10002` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10003` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10004` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10005` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10006` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10007` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10008` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10009` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `throughput_churn:seed10010` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10011` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10012` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10014` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10015` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10016` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10017` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10018` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `throughput_churn:seed10020` | `fixed:large_object` | `fixed:large_object` | `n/a` |

## `adaptive_mix:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.185 | 194979 | 36736 | `large_object` | 0 | 1071.10 | 0.004 | 0.741 |
| `fixed:fragmentation_stable` | 0.273 | 188537 | 39296 | `fragmentation_stable` | 0 | 2479.48 | 0.001 | 1.013 |
| `fixed:balanced` | 0.471 | 204415 | 43648 | `balanced` | 0 | 5639.60 | 0.005 | 0.440 |
| `fixed:cross_thread` | 0.490 | 186018 | 43744 | `cross_thread` | 0 | 5639.60 | 0.005 | 0.440 |
| `fixed:throughput_cache` | 0.506 | 197635 | 44288 | `throughput_cache` | 0 | 6028.53 | 0.005 | 0.440 |
| `fixed:deterministic_latency` | 0.508 | 187929 | 44032 | `deterministic_latency` | 0 | 6028.53 | 0.005 | 0.440 |
| `fixed:compact_rss` | 0.544 | 66161 | 36028 | `compact_rss` | 0 | 109.39 | 0.005 | 0.978 |

## `adaptive_mix:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.199 | 207201 | 37376 | `large_object` | 0 | 1071.10 | 0.004 | 0.764 |
| `fixed:fragmentation_stable` | 0.291 | 196419 | 39808 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.027 |
| `fixed:balanced` | 0.468 | 238356 | 44800 | `balanced` | 0 | 5590.98 | 0.005 | 0.448 |
| `fixed:cross_thread` | 0.532 | 188485 | 44760 | `cross_thread` | 0 | 5590.98 | 0.005 | 0.448 |
| `fixed:compact_rss` | 0.547 | 98593 | 37528 | `compact_rss` | 0 | 109.39 | 0.005 | 0.986 |
| `fixed:throughput_cache` | 0.555 | 195029 | 45440 | `throughput_cache` | 0 | 6077.15 | 0.005 | 0.448 |
| `fixed:deterministic_latency` | 0.560 | 191904 | 45440 | `deterministic_latency` | 0 | 6077.15 | 0.005 | 0.448 |

## `adaptive_mix:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.188 | 192976 | 38400 | `large_object` | 0 | 1071.10 | 0.004 | 0.762 |
| `fixed:fragmentation_stable` | 0.290 | 150659 | 40576 | `fragmentation_stable` | 0 | 2479.48 | 0.001 | 1.025 |
| `fixed:balanced` | 0.458 | 195995 | 44748 | `balanced` | 0 | 5590.98 | 0.004 | 0.454 |
| `fixed:cross_thread` | 0.493 | 165740 | 44876 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.454 |
| `fixed:throughput_cache` | 0.528 | 168129 | 45568 | `throughput_cache` | 0 | 6174.39 | 0.004 | 0.454 |
| `fixed:compact_rss` | 0.548 | 64251 | 38536 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:deterministic_latency` | 0.551 | 148689 | 45440 | `deterministic_latency` | 0 | 6174.39 | 0.004 | 0.454 |

## `adaptive_mix:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.220 | 199447 | 38272 | `large_object` | 0 | 1071.10 | 0.004 | 0.764 |
| `fixed:fragmentation_stable` | 0.326 | 183948 | 40832 | `fragmentation_stable` | 0 | 2576.71 | 0.001 | 1.021 |
| `fixed:balanced` | 0.482 | 234226 | 45308 | `balanced` | 0 | 5688.21 | 0.005 | 0.451 |
| `fixed:compact_rss` | 0.545 | 90852 | 37524 | `compact_rss` | 0 | 109.39 | 0.005 | 0.991 |
| `fixed:deterministic_latency` | 0.553 | 189275 | 45568 | `deterministic_latency` | 0 | 6028.53 | 0.005 | 0.451 |
| `fixed:throughput_cache` | 0.562 | 183010 | 45568 | `throughput_cache` | 0 | 6028.53 | 0.005 | 0.451 |
| `fixed:cross_thread` | 0.600 | 154611 | 45440 | `cross_thread` | 0 | 5688.21 | 0.005 | 0.451 |

## `adaptive_mix:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.224 | 197168 | 37632 | `large_object` | 0 | 1071.10 | 0.004 | 0.757 |
| `fixed:fragmentation_stable` | 0.311 | 171748 | 39808 | `fragmentation_stable` | 0 | 2479.48 | 0.001 | 1.014 |
| `fixed:balanced` | 0.475 | 215369 | 44416 | `balanced` | 0 | 5639.60 | 0.004 | 0.451 |
| `fixed:compact_rss` | 0.546 | 87044 | 37032 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:deterministic_latency` | 0.564 | 168015 | 44672 | `deterministic_latency` | 0 | 6077.15 | 0.004 | 0.451 |
| `fixed:throughput_cache` | 0.578 | 162325 | 44800 | `throughput_cache` | 0 | 6077.15 | 0.004 | 0.451 |
| `fixed:cross_thread` | 0.646 | 125202 | 44416 | `cross_thread` | 0 | 5639.60 | 0.004 | 0.451 |

## `adaptive_mix:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.194 | 217921 | 37760 | `large_object` | 0 | 1071.10 | 0.004 | 0.755 |
| `fixed:fragmentation_stable` | 0.288 | 183723 | 39936 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.019 |
| `fixed:balanced` | 0.477 | 204049 | 43768 | `balanced` | 0 | 5590.98 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.514 | 178507 | 43904 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.452 |
| `fixed:compact_rss` | 0.545 | 83923 | 37476 | `compact_rss` | 0 | 109.39 | 0.004 | 0.989 |
| `fixed:throughput_cache` | 0.566 | 165525 | 44416 | `throughput_cache` | 0 | 6077.15 | 0.004 | 0.452 |
| `fixed:deterministic_latency` | 0.675 | 121319 | 44544 | `deterministic_latency` | 0 | 6077.15 | 0.004 | 0.452 |

## `adaptive_mix:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.189 | 209015 | 37120 | `large_object` | 0 | 1071.10 | 0.004 | 0.746 |
| `fixed:fragmentation_stable` | 0.266 | 194723 | 39424 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 0.995 |
| `fixed:cross_thread` | 0.510 | 189910 | 44032 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.448 |
| `fixed:deterministic_latency` | 0.531 | 188523 | 44416 | `deterministic_latency` | 0 | 5834.06 | 0.004 | 0.448 |
| `fixed:balanced` | 0.536 | 173115 | 43872 | `balanced` | 0 | 5590.98 | 0.004 | 0.448 |
| `fixed:compact_rss` | 0.548 | 93689 | 37076 | `compact_rss` | 0 | 109.39 | 0.004 | 0.986 |
| `fixed:throughput_cache` | 0.570 | 166208 | 44288 | `throughput_cache` | 0 | 5834.06 | 0.004 | 0.448 |

## `adaptive_mix:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.222 | 209222 | 37632 | `large_object` | 0 | 1071.10 | 0.004 | 0.770 |
| `fixed:fragmentation_stable` | 0.311 | 183573 | 40192 | `fragmentation_stable` | 0 | 2625.33 | 0.001 | 1.026 |
| `fixed:balanced` | 0.478 | 243345 | 45184 | `balanced` | 0 | 5736.83 | 0.004 | 0.460 |
| `fixed:cross_thread` | 0.534 | 185944 | 45184 | `cross_thread` | 0 | 5736.83 | 0.004 | 0.460 |
| `fixed:compact_rss` | 0.545 | 82951 | 37400 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:throughput_cache` | 0.558 | 183926 | 45568 | `throughput_cache` | 0 | 6125.77 | 0.004 | 0.460 |
| `fixed:deterministic_latency` | 0.687 | 118820 | 45440 | `deterministic_latency` | 0 | 6125.77 | 0.004 | 0.460 |

## `adaptive_mix:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.199 | 138775 | 38784 | `large_object` | 0 | 1071.10 | 0.004 | 0.770 |
| `fixed:fragmentation_stable` | 0.274 | 125812 | 40960 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.032 |
| `fixed:balanced` | 0.462 | 146405 | 45692 | `balanced` | 0 | 5590.98 | 0.004 | 0.461 |
| `fixed:deterministic_latency` | 0.503 | 142889 | 46336 | `deterministic_latency` | 0 | 6125.77 | 0.004 | 0.461 |
| `fixed:throughput_cache` | 0.510 | 140573 | 46464 | `throughput_cache` | 0 | 6125.77 | 0.004 | 0.461 |
| `fixed:compact_rss` | 0.548 | 59916 | 38936 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:cross_thread` | 0.607 | 91774 | 45720 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.461 |

## `adaptive_mix:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.220 | 195388 | 38272 | `large_object` | 0 | 1071.10 | 0.004 | 0.756 |
| `fixed:fragmentation_stable` | 0.320 | 158339 | 40320 | `fragmentation_stable` | 0 | 2479.48 | 0.001 | 1.016 |
| `fixed:balanced` | 0.454 | 215878 | 44352 | `balanced` | 0 | 5590.98 | 0.004 | 0.457 |
| `fixed:compact_rss` | 0.546 | 80197 | 37788 | `compact_rss` | 0 | 109.39 | 0.004 | 0.995 |
| `fixed:cross_thread` | 0.552 | 148437 | 44508 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.457 |
| `fixed:deterministic_latency` | 0.591 | 150120 | 45440 | `deterministic_latency` | 0 | 6125.77 | 0.004 | 0.457 |
| `fixed:throughput_cache` | 0.678 | 116137 | 45440 | `throughput_cache` | 0 | 6125.77 | 0.004 | 0.457 |

## `adaptive_mix:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.210 | 196194 | 36736 | `large_object` | 0 | 1071.10 | 0.004 | 0.756 |
| `fixed:fragmentation_stable` | 0.407 | 127254 | 39168 | `fragmentation_stable` | 0 | 2528.09 | 0.001 | 1.014 |
| `fixed:balanced` | 0.468 | 217062 | 43520 | `balanced` | 0 | 5688.21 | 0.004 | 0.448 |
| `fixed:throughput_cache` | 0.547 | 173636 | 43648 | `throughput_cache` | 0 | 6223.00 | 0.004 | 0.448 |
| `fixed:compact_rss` | 0.549 | 85784 | 36884 | `compact_rss` | 0 | 109.39 | 0.004 | 0.984 |
| `fixed:deterministic_latency` | 0.564 | 169320 | 44032 | `deterministic_latency` | 0 | 6223.00 | 0.004 | 0.448 |
| `fixed:cross_thread` | 0.605 | 137342 | 43648 | `cross_thread` | 0 | 5688.21 | 0.004 | 0.448 |

## `adaptive_mix:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.232 | 205101 | 38912 | `large_object` | 0 | 1071.10 | 0.004 | 0.760 |
| `fixed:fragmentation_stable` | 0.314 | 181359 | 40960 | `fragmentation_stable` | 0 | 2528.09 | 0.001 | 1.032 |
| `fixed:balanced` | 0.465 | 234015 | 45764 | `balanced` | 0 | 5639.60 | 0.004 | 0.458 |
| `fixed:cross_thread` | 0.532 | 181740 | 45660 | `cross_thread` | 0 | 5639.60 | 0.004 | 0.458 |
| `fixed:compact_rss` | 0.544 | 95096 | 38348 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:deterministic_latency` | 0.571 | 180239 | 46592 | `deterministic_latency` | 0 | 6077.15 | 0.004 | 0.458 |
| `fixed:throughput_cache` | 0.577 | 177223 | 46592 | `throughput_cache` | 0 | 6077.15 | 0.004 | 0.458 |

## `adaptive_mix:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.217 | 207890 | 37632 | `large_object` | 0 | 1071.10 | 0.004 | 0.764 |
| `fixed:fragmentation_stable` | 0.271 | 204566 | 39936 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.009 |
| `fixed:balanced` | 0.451 | 227801 | 43804 | `balanced` | 0 | 5590.98 | 0.004 | 0.462 |
| `fixed:cross_thread` | 0.504 | 186571 | 43820 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.462 |
| `fixed:compact_rss` | 0.548 | 92062 | 37384 | `compact_rss` | 0 | 109.39 | 0.004 | 0.998 |
| `fixed:deterministic_latency` | 0.597 | 161555 | 45056 | `deterministic_latency` | 0 | 6077.15 | 0.004 | 0.462 |
| `fixed:throughput_cache` | 0.691 | 126330 | 45056 | `throughput_cache` | 0 | 6077.15 | 0.004 | 0.462 |

## `adaptive_mix:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.188 | 221810 | 36224 | `large_object` | 0 | 1071.10 | 0.004 | 0.748 |
| `fixed:fragmentation_stable` | 0.281 | 195782 | 38400 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.008 |
| `fixed:balanced` | 0.479 | 222063 | 42880 | `balanced` | 0 | 5590.98 | 0.004 | 0.445 |
| `fixed:cross_thread` | 0.536 | 185004 | 42880 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.445 |
| `fixed:deterministic_latency` | 0.538 | 191324 | 43008 | `deterministic_latency` | 0 | 5882.68 | 0.004 | 0.445 |
| `fixed:compact_rss` | 0.546 | 99125 | 36196 | `compact_rss` | 0 | 109.39 | 0.004 | 0.983 |
| `fixed:throughput_cache` | 0.561 | 182406 | 43264 | `throughput_cache` | 0 | 5882.68 | 0.004 | 0.445 |

## `adaptive_mix:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.198 | 213235 | 36608 | `large_object` | 0 | 1071.10 | 0.004 | 0.751 |
| `fixed:fragmentation_stable` | 0.295 | 199181 | 39040 | `fragmentation_stable` | 0 | 2528.09 | 0.001 | 1.024 |
| `fixed:balanced` | 0.472 | 238186 | 43512 | `balanced` | 0 | 5639.60 | 0.005 | 0.445 |
| `fixed:cross_thread` | 0.527 | 185986 | 43520 | `cross_thread` | 0 | 5639.60 | 0.005 | 0.445 |
| `fixed:compact_rss` | 0.543 | 85101 | 36028 | `compact_rss` | 0 | 109.39 | 0.005 | 0.983 |
| `fixed:throughput_cache` | 0.564 | 176845 | 44032 | `throughput_cache` | 0 | 5979.92 | 0.005 | 0.445 |
| `fixed:deterministic_latency` | 0.708 | 115180 | 44160 | `deterministic_latency` | 0 | 5979.92 | 0.005 | 0.445 |

## `adaptive_mix:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.216 | 199936 | 37760 | `large_object` | 0 | 1071.10 | 0.004 | 0.752 |
| `fixed:fragmentation_stable` | 0.303 | 172437 | 40064 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.011 |
| `fixed:balanced` | 0.467 | 231877 | 44100 | `balanced` | 0 | 5590.98 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.516 | 178196 | 44128 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.452 |
| `fixed:compact_rss` | 0.546 | 72826 | 37608 | `compact_rss` | 0 | 109.39 | 0.004 | 0.989 |
| `fixed:deterministic_latency` | 0.554 | 173599 | 44800 | `deterministic_latency` | 0 | 5979.92 | 0.004 | 0.452 |
| `fixed:throughput_cache` | 0.554 | 173326 | 44800 | `throughput_cache` | 0 | 5979.92 | 0.004 | 0.452 |

## `adaptive_mix:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.196 | 203133 | 37120 | `large_object` | 0 | 1071.10 | 0.004 | 0.747 |
| `fixed:fragmentation_stable` | 0.305 | 164674 | 39296 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.009 |
| `fixed:deterministic_latency` | 0.529 | 179464 | 44416 | `deterministic_latency` | 0 | 5882.68 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.537 | 165052 | 44160 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.453 |
| `fixed:throughput_cache` | 0.544 | 173055 | 44544 | `throughput_cache` | 0 | 5931.30 | 0.004 | 0.452 |
| `fixed:compact_rss` | 0.547 | 85562 | 36740 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:balanced` | 0.581 | 144732 | 44160 | `balanced` | 0 | 5590.98 | 0.004 | 0.453 |

## `adaptive_mix:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.211 | 215648 | 37248 | `large_object` | 0 | 1071.10 | 0.004 | 0.748 |
| `fixed:fragmentation_stable` | 0.296 | 190655 | 39552 | `fragmentation_stable` | 0 | 2479.48 | 0.001 | 1.007 |
| `fixed:balanced` | 0.472 | 236427 | 44032 | `balanced` | 0 | 5639.60 | 0.004 | 0.444 |
| `fixed:cross_thread` | 0.524 | 188486 | 44032 | `cross_thread` | 0 | 5639.60 | 0.004 | 0.444 |
| `fixed:compact_rss` | 0.545 | 87825 | 37068 | `compact_rss` | 0 | 109.39 | 0.004 | 0.981 |
| `fixed:throughput_cache` | 0.561 | 179949 | 44416 | `throughput_cache` | 0 | 6077.15 | 0.004 | 0.444 |
| `fixed:deterministic_latency` | 0.706 | 118350 | 44544 | `deterministic_latency` | 0 | 6077.15 | 0.004 | 0.444 |

## `adaptive_mix:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.258 | 165131 | 38016 | `large_object` | 0 | 1071.10 | 0.004 | 0.746 |
| `fixed:fragmentation_stable` | 0.362 | 142674 | 40448 | `fragmentation_stable` | 0 | 2479.48 | 0.001 | 1.023 |
| `fixed:balanced` | 0.466 | 215170 | 44032 | `balanced` | 0 | 5639.60 | 0.004 | 0.450 |
| `fixed:compact_rss` | 0.544 | 78272 | 37532 | `compact_rss` | 0 | 109.39 | 0.004 | 0.989 |
| `fixed:throughput_cache` | 0.565 | 162670 | 44672 | `throughput_cache` | 0 | 6125.77 | 0.004 | 0.450 |
| `fixed:deterministic_latency` | 0.571 | 157021 | 44544 | `deterministic_latency` | 0 | 6125.77 | 0.004 | 0.450 |
| `fixed:cross_thread` | 0.648 | 113593 | 44136 | `cross_thread` | 0 | 5639.60 | 0.004 | 0.450 |

## `adaptive_mix:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.203 | 200325 | 36608 | `large_object` | 0 | 1071.10 | 0.004 | 0.748 |
| `fixed:fragmentation_stable` | 0.302 | 174143 | 38784 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.015 |
| `fixed:balanced` | 0.469 | 241316 | 43092 | `balanced` | 0 | 5590.98 | 0.005 | 0.446 |
| `fixed:cross_thread` | 0.519 | 176822 | 43116 | `cross_thread` | 0 | 5590.98 | 0.005 | 0.446 |
| `fixed:throughput_cache` | 0.536 | 190863 | 43776 | `throughput_cache` | 0 | 5979.92 | 0.005 | 0.446 |
| `fixed:compact_rss` | 0.545 | 67181 | 36036 | `compact_rss` | 0 | 109.39 | 0.005 | 0.985 |
| `fixed:deterministic_latency` | 0.546 | 179882 | 43776 | `deterministic_latency` | 0 | 5979.92 | 0.005 | 0.446 |

## `fragmentation_drift:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.129 | 128123 | 47872 | `large_object` | 0 | 314.68 | 0.000 | 12.098 |
| `fixed:compact_rss` | 0.287 | 120870 | 46972 | `compact_rss` | 0 | 108.86 | 0.002 | 9.483 |
| `fixed:fragmentation_stable` | 0.615 | 115039 | 50432 | `fragmentation_stable` | 0 | 1891.51 | 0.001 | 11.109 |
| `fixed:cross_thread` | 0.699 | 119325 | 54968 | `cross_thread` | 0 | 4721.97 | 0.002 | 9.423 |
| `fixed:throughput_cache` | 0.712 | 119870 | 55680 | `throughput_cache` | 0 | 4994.13 | 0.002 | 9.423 |
| `fixed:balanced` | 0.734 | 117993 | 54864 | `balanced` | 0 | 4721.97 | 0.002 | 9.423 |
| `fixed:deterministic_latency` | 0.829 | 115536 | 55424 | `deterministic_latency` | 0 | 4994.13 | 0.002 | 9.423 |

## `fragmentation_drift:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.141 | 134048 | 48000 | `large_object` | 0 | 301.08 | 0.000 | 12.558 |
| `fixed:fragmentation_stable` | 0.327 | 124067 | 50560 | `fragmentation_stable` | 0 | 1823.47 | 0.001 | 11.634 |
| `fixed:compact_rss` | 0.452 | 90074 | 47116 | `compact_rss` | 0 | 108.86 | 0.002 | 9.681 |
| `fixed:cross_thread` | 0.457 | 136957 | 55108 | `cross_thread` | 0 | 5034.95 | 0.002 | 9.625 |
| `fixed:balanced` | 0.523 | 124950 | 55188 | `balanced` | 0 | 5034.95 | 0.002 | 9.625 |
| `fixed:throughput_cache` | 0.578 | 122209 | 56320 | `throughput_cache` | 0 | 5415.97 | 0.002 | 9.625 |
| `fixed:deterministic_latency` | 0.595 | 119976 | 56448 | `deterministic_latency` | 0 | 5415.97 | 0.002 | 9.625 |

## `fragmentation_drift:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.160 | 130711 | 48384 | `large_object` | 0 | 260.25 | 0.000 | 12.428 |
| `fixed:fragmentation_stable` | 0.331 | 129154 | 50688 | `fragmentation_stable` | 0 | 1837.08 | 0.001 | 11.502 |
| `fixed:compact_rss` | 0.369 | 121844 | 47672 | `compact_rss` | 0 | 108.86 | 0.002 | 9.660 |
| `fixed:cross_thread` | 0.444 | 132288 | 54640 | `cross_thread` | 0 | 4721.97 | 0.002 | 9.630 |
| `fixed:throughput_cache` | 0.544 | 130465 | 56192 | `throughput_cache` | 0 | 5211.85 | 0.002 | 9.630 |
| `fixed:deterministic_latency` | 0.677 | 125084 | 56064 | `deterministic_latency` | 0 | 5211.85 | 0.002 | 9.630 |
| `fixed:balanced` | 0.796 | 118948 | 54724 | `balanced` | 0 | 4721.97 | 0.002 | 9.630 |

## `fragmentation_drift:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.136 | 142426 | 47636 | `compact_rss` | 0 | 108.86 | 0.002 | 9.430 |
| `fixed:large_object` | 0.205 | 135783 | 48512 | `large_object` | 0 | 369.12 | 0.000 | 12.243 |
| `fixed:throughput_cache` | 0.497 | 148385 | 56320 | `throughput_cache` | 0 | 4994.13 | 0.002 | 9.343 |
| `fixed:deterministic_latency` | 0.506 | 147283 | 56448 | `deterministic_latency` | 0 | 4994.13 | 0.002 | 9.343 |
| `fixed:balanced` | 0.602 | 128042 | 55756 | `balanced` | 0 | 4776.40 | 0.002 | 9.343 |
| `fixed:cross_thread` | 0.621 | 125460 | 55772 | `cross_thread` | 0 | 4776.40 | 0.002 | 9.343 |
| `fixed:fragmentation_stable` | 0.626 | 103109 | 51328 | `fragmentation_stable` | 0 | 2054.80 | 0.001 | 11.155 |

## `fragmentation_drift:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.103 | 138645 | 47012 | `compact_rss` | 0 | 108.86 | 0.002 | 9.638 |
| `fixed:large_object` | 0.264 | 115875 | 48000 | `large_object` | 0 | 341.90 | 0.000 | 12.338 |
| `fixed:fragmentation_stable` | 0.298 | 131513 | 50304 | `fragmentation_stable` | 0 | 1864.29 | 0.001 | 11.311 |
| `fixed:cross_thread` | 0.481 | 135349 | 54504 | `cross_thread` | 0 | 4749.18 | 0.002 | 9.558 |
| `fixed:deterministic_latency` | 0.543 | 130257 | 55424 | `deterministic_latency` | 0 | 5075.77 | 0.002 | 9.558 |
| `fixed:throughput_cache` | 0.692 | 107653 | 55424 | `throughput_cache` | 0 | 5075.77 | 0.002 | 9.558 |
| `fixed:balanced` | 0.816 | 90933 | 54528 | `balanced` | 0 | 4749.18 | 0.002 | 9.558 |

## `fragmentation_drift:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.219 | 112207 | 47472 | `compact_rss` | 0 | 108.86 | 0.002 | 9.694 |
| `fixed:large_object` | 0.340 | 96233 | 48128 | `large_object` | 0 | 273.86 | 0.000 | 12.347 |
| `fixed:fragmentation_stable` | 0.425 | 103002 | 50176 | `fragmentation_stable` | 0 | 1741.82 | 0.001 | 11.492 |
| `fixed:balanced` | 0.453 | 139251 | 54108 | `balanced` | 0 | 4572.28 | 0.002 | 9.634 |
| `fixed:throughput_cache` | 0.501 | 137954 | 55296 | `throughput_cache` | 0 | 4898.87 | 0.002 | 9.634 |
| `fixed:deterministic_latency` | 0.503 | 138454 | 55424 | `deterministic_latency` | 0 | 4898.87 | 0.002 | 9.634 |
| `fixed:cross_thread` | 0.802 | 80775 | 54072 | `cross_thread` | 0 | 4572.28 | 0.002 | 9.634 |

## `fragmentation_drift:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.120 | 131394 | 48768 | `large_object` | 0 | 246.65 | 0.000 | 12.334 |
| `fixed:compact_rss` | 0.253 | 105034 | 48196 | `compact_rss` | 0 | 108.86 | 0.002 | 9.802 |
| `fixed:fragmentation_stable` | 0.335 | 116255 | 51072 | `fragmentation_stable` | 0 | 1769.04 | 0.001 | 11.449 |
| `fixed:cross_thread` | 0.481 | 131010 | 55680 | `cross_thread` | 0 | 4762.79 | 0.002 | 9.723 |
| `fixed:throughput_cache` | 0.504 | 129804 | 56192 | `throughput_cache` | 0 | 4871.65 | 0.002 | 9.723 |
| `fixed:balanced` | 0.534 | 120998 | 55808 | `balanced` | 0 | 4762.79 | 0.002 | 9.723 |
| `fixed:deterministic_latency` | 0.850 | 82956 | 56320 | `deterministic_latency` | 0 | 4871.65 | 0.002 | 9.723 |

## `fragmentation_drift:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.127 | 141200 | 47268 | `compact_rss` | 0 | 108.86 | 0.002 | 9.642 |
| `fixed:fragmentation_stable` | 0.416 | 130288 | 50432 | `fragmentation_stable` | 0 | 1973.16 | 0.001 | 11.438 |
| `fixed:large_object` | 0.466 | 116692 | 47744 | `large_object` | 0 | 233.04 | 0.000 | 12.432 |
| `fixed:deterministic_latency` | 0.500 | 143498 | 56064 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.585 |
| `fixed:cross_thread` | 0.524 | 138993 | 55324 | `cross_thread` | 0 | 4912.48 | 0.002 | 9.585 |
| `fixed:balanced` | 0.544 | 137081 | 55224 | `balanced` | 0 | 4912.48 | 0.002 | 9.585 |
| `fixed:throughput_cache` | 0.565 | 137610 | 56064 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.585 |

## `fragmentation_drift:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.121 | 130421 | 48384 | `large_object` | 0 | 260.25 | 0.000 | 12.423 |
| `fixed:compact_rss` | 0.206 | 113465 | 47700 | `compact_rss` | 0 | 108.86 | 0.002 | 9.640 |
| `fixed:fragmentation_stable` | 0.338 | 116078 | 50688 | `fragmentation_stable` | 0 | 1782.64 | 0.001 | 11.492 |
| `fixed:deterministic_latency` | 0.555 | 120467 | 56448 | `deterministic_latency` | 0 | 5102.99 | 0.002 | 9.583 |
| `fixed:cross_thread` | 0.559 | 112634 | 55108 | `cross_thread` | 0 | 4667.53 | 0.002 | 9.583 |
| `fixed:throughput_cache` | 0.589 | 115646 | 56576 | `throughput_cache` | 0 | 5102.99 | 0.002 | 9.583 |
| `fixed:balanced` | 0.796 | 86755 | 54960 | `balanced` | 0 | 4667.53 | 0.002 | 9.583 |

## `fragmentation_drift:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.164 | 111676 | 47520 | `compact_rss` | 0 | 108.86 | 0.002 | 9.660 |
| `fixed:large_object` | 0.240 | 101295 | 48384 | `large_object` | 0 | 341.90 | 0.000 | 12.264 |
| `fixed:balanced` | 0.478 | 123041 | 55088 | `balanced` | 0 | 4749.18 | 0.002 | 9.596 |
| `fixed:throughput_cache` | 0.497 | 128513 | 56064 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.596 |
| `fixed:deterministic_latency` | 0.541 | 116775 | 56192 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.596 |
| `fixed:cross_thread` | 0.602 | 94850 | 54968 | `cross_thread` | 0 | 4749.18 | 0.002 | 9.596 |
| `fixed:fragmentation_stable` | 0.607 | 69412 | 50432 | `fragmentation_stable` | 0 | 1864.29 | 0.001 | 11.464 |

## `fragmentation_drift:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.102 | 148671 | 46860 | `compact_rss` | 0 | 108.86 | 0.002 | 9.421 |
| `fixed:fragmentation_stable` | 0.329 | 131600 | 49792 | `fragmentation_stable` | 0 | 1891.51 | 0.001 | 11.291 |
| `fixed:large_object` | 0.468 | 90258 | 47360 | `large_object` | 0 | 260.25 | 0.000 | 12.179 |
| `fixed:balanced` | 0.504 | 135310 | 53836 | `balanced` | 0 | 4776.40 | 0.002 | 9.375 |
| `fixed:cross_thread` | 0.507 | 135144 | 53936 | `cross_thread` | 0 | 4776.40 | 0.002 | 9.375 |
| `fixed:deterministic_latency` | 0.536 | 139343 | 55168 | `deterministic_latency` | 0 | 5211.85 | 0.002 | 9.375 |
| `fixed:throughput_cache` | 0.576 | 129646 | 55040 | `throughput_cache` | 0 | 5211.85 | 0.002 | 9.375 |

## `fragmentation_drift:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.225 | 107055 | 47800 | `compact_rss` | 0 | 108.86 | 0.002 | 9.817 |
| `fixed:large_object` | 0.283 | 103499 | 48768 | `large_object` | 0 | 314.68 | 0.000 | 12.438 |
| `fixed:fragmentation_stable` | 0.395 | 104951 | 50688 | `fragmentation_stable` | 0 | 1837.08 | 0.001 | 11.683 |
| `fixed:throughput_cache` | 0.497 | 124996 | 56448 | `throughput_cache` | 0 | 5266.29 | 0.002 | 9.764 |
| `fixed:balanced` | 0.524 | 114773 | 55328 | `balanced` | 0 | 4939.69 | 0.002 | 9.764 |
| `fixed:cross_thread` | 0.543 | 112169 | 55320 | `cross_thread` | 0 | 4939.69 | 0.002 | 9.764 |
| `fixed:deterministic_latency` | 0.850 | 84680 | 56576 | `deterministic_latency` | 0 | 5266.29 | 0.002 | 9.764 |

## `fragmentation_drift:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.129 | 140771 | 48132 | `compact_rss` | 0 | 108.86 | 0.002 | 9.817 |
| `fixed:fragmentation_stable` | 0.292 | 139378 | 50816 | `fragmentation_stable` | 0 | 1769.04 | 0.001 | 11.523 |
| `fixed:cross_thread` | 0.451 | 143100 | 54940 | `cross_thread` | 0 | 4708.36 | 0.002 | 9.772 |
| `fixed:large_object` | 0.471 | 118140 | 48768 | `large_object` | 0 | 246.65 | 0.000 | 12.442 |
| `fixed:balanced` | 0.525 | 137034 | 54960 | `balanced` | 0 | 4708.36 | 0.002 | 9.772 |
| `fixed:throughput_cache` | 0.531 | 140191 | 56192 | `throughput_cache` | 0 | 5089.38 | 0.002 | 9.772 |
| `fixed:deterministic_latency` | 0.579 | 136623 | 56320 | `deterministic_latency` | 0 | 5089.38 | 0.002 | 9.772 |

## `fragmentation_drift:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.128 | 133668 | 48256 | `large_object` | 0 | 287.47 | 0.000 | 12.413 |
| `fixed:compact_rss` | 0.206 | 119156 | 47412 | `compact_rss` | 0 | 108.86 | 0.002 | 9.760 |
| `fixed:fragmentation_stable` | 0.385 | 116038 | 50304 | `fragmentation_stable` | 0 | 1755.43 | 0.001 | 11.457 |
| `fixed:cross_thread` | 0.483 | 133611 | 55136 | `cross_thread` | 0 | 4640.32 | 0.002 | 9.677 |
| `fixed:balanced` | 0.532 | 126176 | 55120 | `balanced` | 0 | 4640.32 | 0.002 | 9.677 |
| `fixed:deterministic_latency` | 0.622 | 116502 | 55552 | `deterministic_latency` | 0 | 4749.18 | 0.002 | 9.677 |
| `fixed:throughput_cache` | 0.850 | 94607 | 55680 | `throughput_cache` | 0 | 4749.18 | 0.002 | 9.677 |

## `fragmentation_drift:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.233 | 102513 | 48512 | `large_object` | 0 | 328.29 | 0.000 | 12.323 |
| `fixed:compact_rss` | 0.317 | 81923 | 47556 | `compact_rss` | 0 | 108.86 | 0.002 | 9.598 |
| `fixed:balanced` | 0.476 | 133540 | 55680 | `balanced` | 0 | 4898.87 | 0.002 | 9.538 |
| `fixed:throughput_cache` | 0.515 | 126987 | 56320 | `throughput_cache` | 0 | 5062.17 | 0.002 | 9.538 |
| `fixed:deterministic_latency` | 0.565 | 112095 | 56448 | `deterministic_latency` | 0 | 5062.17 | 0.002 | 9.538 |
| `fixed:fragmentation_stable` | 0.616 | 65822 | 50944 | `fragmentation_stable` | 0 | 1959.55 | 0.001 | 11.349 |
| `fixed:cross_thread` | 0.742 | 74961 | 55672 | `cross_thread` | 0 | 4898.87 | 0.002 | 9.538 |

## `fragmentation_drift:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.102 | 143168 | 47420 | `compact_rss` | 0 | 108.86 | 0.002 | 9.675 |
| `fixed:large_object` | 0.284 | 135897 | 48384 | `large_object` | 0 | 287.47 | 0.000 | 12.306 |
| `fixed:fragmentation_stable` | 0.350 | 138849 | 50560 | `fragmentation_stable` | 0 | 1809.86 | 0.001 | 11.460 |
| `fixed:cross_thread` | 0.606 | 136123 | 54764 | `cross_thread` | 0 | 4694.75 | 0.002 | 9.608 |
| `fixed:deterministic_latency` | 0.607 | 137923 | 56064 | `deterministic_latency` | 0 | 4966.91 | 0.002 | 9.608 |
| `fixed:throughput_cache` | 0.624 | 137285 | 56192 | `throughput_cache` | 0 | 4966.91 | 0.002 | 9.608 |
| `fixed:balanced` | 0.806 | 127697 | 54772 | `balanced` | 0 | 4694.75 | 0.002 | 9.608 |

## `fragmentation_drift:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.102 | 125511 | 47096 | `compact_rss` | 0 | 108.86 | 0.002 | 9.500 |
| `fixed:large_object` | 0.147 | 120985 | 47744 | `large_object` | 0 | 301.08 | 0.000 | 11.981 |
| `fixed:fragmentation_stable` | 0.269 | 124378 | 50176 | `fragmentation_stable` | 0 | 1823.47 | 0.001 | 11.198 |
| `fixed:cross_thread` | 0.487 | 123083 | 54784 | `cross_thread` | 0 | 4762.79 | 0.002 | 9.455 |
| `fixed:balanced` | 0.495 | 121460 | 54760 | `balanced` | 0 | 4762.79 | 0.002 | 9.455 |
| `fixed:deterministic_latency` | 0.524 | 120297 | 55424 | `deterministic_latency` | 0 | 4926.09 | 0.002 | 9.455 |
| `fixed:throughput_cache` | 0.850 | 80674 | 55552 | `throughput_cache` | 0 | 4926.09 | 0.002 | 9.455 |

## `fragmentation_drift:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.219 | 113928 | 47820 | `compact_rss` | 0 | 108.86 | 0.002 | 9.672 |
| `fixed:fragmentation_stable` | 0.383 | 112765 | 50944 | `fragmentation_stable` | 0 | 1823.47 | 0.001 | 11.385 |
| `fixed:balanced` | 0.473 | 131877 | 55016 | `balanced` | 0 | 4817.22 | 0.002 | 9.623 |
| `fixed:large_object` | 0.477 | 87165 | 48640 | `large_object` | 0 | 301.08 | 0.000 | 12.349 |
| `fixed:deterministic_latency` | 0.494 | 134828 | 55936 | `deterministic_latency` | 0 | 5143.81 | 0.002 | 9.623 |
| `fixed:throughput_cache` | 0.535 | 127789 | 56192 | `throughput_cache` | 0 | 5143.81 | 0.002 | 9.623 |
| `fixed:cross_thread` | 0.566 | 115252 | 54956 | `cross_thread` | 0 | 4817.22 | 0.002 | 9.623 |

## `fragmentation_drift:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.143 | 132410 | 48148 | `compact_rss` | 0 | 108.86 | 0.002 | 9.570 |
| `fixed:large_object` | 0.170 | 132334 | 49024 | `large_object` | 0 | 287.47 | 0.000 | 12.062 |
| `fixed:fragmentation_stable` | 0.294 | 135092 | 51328 | `fragmentation_stable` | 0 | 1864.29 | 0.001 | 11.255 |
| `fixed:cross_thread` | 0.458 | 139907 | 54880 | `cross_thread` | 0 | 4858.05 | 0.002 | 9.491 |
| `fixed:balanced` | 0.502 | 131623 | 54860 | `balanced` | 0 | 4858.05 | 0.002 | 9.491 |
| `fixed:deterministic_latency` | 0.571 | 127202 | 55936 | `deterministic_latency` | 0 | 5239.07 | 0.002 | 9.491 |
| `fixed:throughput_cache` | 0.850 | 93677 | 55936 | `throughput_cache` | 0 | 5239.07 | 0.002 | 9.491 |

## `fragmentation_drift:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.153 | 134557 | 47664 | `compact_rss` | 0 | 108.86 | 0.002 | 9.481 |
| `fixed:large_object` | 0.177 | 133332 | 48256 | `large_object` | 0 | 287.47 | 0.000 | 12.113 |
| `fixed:deterministic_latency` | 0.500 | 146659 | 56064 | `deterministic_latency` | 0 | 5075.77 | 0.002 | 9.406 |
| `fixed:throughput_cache` | 0.509 | 142840 | 55808 | `throughput_cache` | 0 | 5075.77 | 0.002 | 9.406 |
| `fixed:fragmentation_stable` | 0.599 | 90362 | 50432 | `fragmentation_stable` | 0 | 1755.43 | 0.001 | 11.219 |
| `fixed:cross_thread` | 0.604 | 117947 | 55140 | `cross_thread` | 0 | 4803.61 | 0.002 | 9.406 |
| `fixed:balanced` | 0.610 | 116895 | 55124 | `balanced` | 0 | 4803.61 | 0.002 | 9.406 |

## `large_burst:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.201 | 398423 | 14080 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.222 | 302988 | 14080 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.235 | 264743 | 14080 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.259 | 356463 | 14208 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.327 | 405120 | 14208 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:cross_thread` | 0.418 | 316838 | 14208 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.650 | 63632 | 14208 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.209 | 305416 | 14464 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.251 | 333395 | 14592 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.327 | 341149 | 14592 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:cross_thread` | 0.406 | 314362 | 14592 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.414 | 288528 | 14592 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.422 | 265208 | 14592 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.650 | 60587 | 14592 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.207 | 375611 | 14976 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.255 | 382972 | 15104 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.327 | 414322 | 15104 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:cross_thread` | 0.416 | 332244 | 15104 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.423 | 305528 | 15104 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.431 | 278532 | 15104 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.650 | 64053 | 15104 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.233 | 268879 | 15360 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.250 | 410029 | 15488 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.329 | 399576 | 15488 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:cross_thread` | 0.425 | 293996 | 15488 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.430 | 277773 | 15488 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.437 | 259761 | 15488 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.650 | 62935 | 15488 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.130 | 368060 | 15744 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.200 | 380947 | 15744 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.223 | 286296 | 15744 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.227 | 273286 | 15744 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.254 | 356913 | 15872 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:cross_thread` | 0.428 | 270932 | 15872 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 62331 | 15744 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.200 | 389601 | 16128 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.228 | 276342 | 16128 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.335 | 349038 | 16256 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:large_object` | 0.360 | 149832 | 16256 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:deterministic_latency` | 0.415 | 318268 | 16256 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.439 | 249235 | 16256 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.650 | 63951 | 16256 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.213 | 312990 | 16512 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.259 | 327403 | 16640 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.327 | 373193 | 16640 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:throughput_cache` | 0.426 | 267503 | 16640 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.428 | 261504 | 16640 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.433 | 250747 | 16640 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.650 | 59661 | 16640 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.127 | 417941 | 17024 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:large_object` | 0.157 | 156111 | 17024 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:balanced` | 0.223 | 308574 | 17024 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.228 | 290318 | 17024 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.235 | 269281 | 17024 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.236 | 267209 | 17024 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 64508 | 17024 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.050 | 402019 | 17408 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.134 | 361550 | 17408 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.210 | 342992 | 17408 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.219 | 301710 | 17408 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.232 | 256347 | 17408 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.233 | 253742 | 17408 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 56388 | 17408 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.056 | 347191 | 17792 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.135 | 335485 | 17792 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.200 | 390941 | 17792 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.217 | 288112 | 17792 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.222 | 265304 | 17792 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.226 | 250302 | 17792 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 46220 | 17792 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.136 | 340203 | 18176 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:large_object` | 0.159 | 141444 | 18176 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:balanced` | 0.200 | 391459 | 18176 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.217 | 306710 | 18176 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.229 | 267652 | 18176 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.258 | 202234 | 18176 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 58994 | 18176 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 243861 | 18560 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.164 | 205464 | 18560 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.200 | 332331 | 18560 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.213 | 271755 | 18560 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.214 | 270501 | 18560 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.239 | 201633 | 18560 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 48253 | 18560 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.060 | 314115 | 19072 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.131 | 349501 | 19072 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.200 | 378683 | 19072 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.210 | 314493 | 19072 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.211 | 311762 | 19072 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.224 | 254173 | 19072 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 46960 | 19072 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.050 | 352339 | 19328 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.127 | 350857 | 19328 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.206 | 322060 | 19328 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.213 | 291250 | 19328 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.237 | 221552 | 19328 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.238 | 219563 | 19328 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 53505 | 19328 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.050 | 350991 | 19840 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.127 | 350224 | 19840 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.206 | 308353 | 19840 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.210 | 286557 | 19840 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.213 | 271249 | 19840 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.223 | 233272 | 19840 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 40193 | 19840 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.060 | 349779 | 20096 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.127 | 404078 | 20096 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:deterministic_latency` | 0.219 | 311340 | 20096 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.219 | 311229 | 20096 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.219 | 310837 | 20096 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.224 | 295378 | 20096 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 63114 | 20096 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.050 | 378480 | 20480 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.127 | 380406 | 20480 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.205 | 355595 | 20480 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.220 | 294148 | 20480 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.223 | 282003 | 20480 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.238 | 242949 | 20480 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 61044 | 20480 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.055 | 342466 | 20736 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:balanced` | 0.200 | 369274 | 20736 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.216 | 298432 | 20736 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.217 | 295640 | 20736 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.232 | 146231 | 20736 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:cross_thread` | 0.240 | 233866 | 20736 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 60529 | 20736 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.050 | 353497 | 21120 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.142 | 290816 | 21120 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.200 | 352671 | 21120 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.220 | 272536 | 21120 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.227 | 253900 | 21120 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.232 | 241868 | 21120 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 58168 | 21120 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.054 | 298905 | 21504 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.127 | 319633 | 21504 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `fixed:balanced` | 0.202 | 308443 | 21504 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.213 | 260299 | 21504 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.217 | 246296 | 21504 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.263 | 153489 | 21504 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.450 | 45759 | 21504 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `latency_loop:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 8147436 | 14336 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.149 | 7940469 | 14336 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.201 | 7861014 | 14336 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.202 | 7175360 | 14336 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.338 | 785965 | 14336 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.351 | 723292 | 14336 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.550 | 328837 | 14336 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 8241117 | 14720 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.149 | 8212166 | 14720 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.202 | 7810833 | 14720 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.215 | 5751219 | 14720 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.543 | 763953 | 14720 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.546 | 758984 | 14720 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.550 | 750842 | 14720 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.001 | 7694570 | 15232 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.150 | 7506464 | 15232 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 8150755 | 15232 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.202 | 7405345 | 15232 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.364 | 757749 | 15232 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.460 | 495266 | 15232 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.550 | 373122 | 15232 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.002 | 8299392 | 15616 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.148 | 8714999 | 15616 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.203 | 8085331 | 15616 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.211 | 6639125 | 15616 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.525 | 834643 | 15616 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.525 | 834585 | 15616 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.550 | 779280 | 15616 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.002 | 6987531 | 16000 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.152 | 6861246 | 16000 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 7485596 | 16000 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.201 | 7327390 | 16000 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.528 | 721954 | 16000 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.532 | 715620 | 16000 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.550 | 681581 | 16000 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 7540192 | 16384 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.150 | 7081414 | 16384 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 7441708 | 16384 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.212 | 5197845 | 16384 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.457 | 694493 | 16384 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.516 | 576194 | 16384 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.550 | 523427 | 16384 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 8550336 | 16768 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.149 | 8323515 | 16768 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 8532196 | 16768 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.205 | 7521052 | 16768 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.522 | 854412 | 16768 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.532 | 829982 | 16768 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.550 | 791720 | 16768 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.039 | 1928607 | 17152 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.149 | 7679645 | 17152 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 7829459 | 17152 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.201 | 7365519 | 17152 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.325 | 730750 | 17152 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.325 | 730482 | 17152 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.550 | 276685 | 17152 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.007 | 7142717 | 17536 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.148 | 8579922 | 17536 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.201 | 8445985 | 17536 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.201 | 8370078 | 17536 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.531 | 794285 | 17536 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.533 | 790813 | 17536 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.550 | 755333 | 17536 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.150 | 7240139 | 17920 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:large_object` | 0.200 | 7565488 | 18048 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.203 | 6974133 | 17920 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.212 | 5468999 | 17920 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.489 | 716200 | 17920 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.500 | 691208 | 17920 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.550 | 600398 | 17920 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.200 | 8670391 | 18432 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:balanced` | 0.205 | 7587901 | 18304 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.353 | 7679766 | 18432 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.410 | 6672363 | 18432 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.550 | 765168 | 18304 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.721 | 827658 | 18432 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.732 | 801985 | 18432 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.200 | 8764453 | 18816 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:balanced` | 0.203 | 8054852 | 18688 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.349 | 8499522 | 18816 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.402 | 8211971 | 18816 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.522 | 801377 | 18688 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.709 | 833717 | 18816 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.750 | 743653 | 18816 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 7604985 | 19200 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.149 | 7191706 | 19200 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.201 | 7349878 | 19200 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.261 | 1822629 | 19200 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.387 | 703459 | 19200 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.397 | 670899 | 19200 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.550 | 393265 | 19200 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.150 | 7896499 | 19456 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 8395611 | 19456 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.201 | 8006190 | 19712 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.402 | 7957857 | 19712 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.473 | 836003 | 19456 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.499 | 771204 | 19456 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.750 | 667441 | 19712 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 8102528 | 19840 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.148 | 8166550 | 19840 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.201 | 7854622 | 19840 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.203 | 7590758 | 19840 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.534 | 811673 | 19840 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.546 | 784519 | 19840 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.550 | 777224 | 19840 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.006 | 7706041 | 20352 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.148 | 9024699 | 20352 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.202 | 8609203 | 20352 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.210 | 6970120 | 20352 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.520 | 858318 | 20352 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.521 | 855145 | 20352 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.550 | 790889 | 20352 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 8174329 | 20608 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.154 | 6951670 | 20608 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.202 | 7626548 | 20608 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.208 | 6641166 | 20608 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.525 | 755544 | 20608 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.533 | 738514 | 20608 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.550 | 706363 | 20608 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.004 | 6388974 | 20864 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.154 | 6128197 | 20864 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 7250031 | 20864 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.217 | 4674937 | 20864 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.483 | 729389 | 20864 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.505 | 682691 | 20864 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.550 | 601643 | 20864 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 8518546 | 21376 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.151 | 7831047 | 21376 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 8446256 | 21376 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.201 | 8383152 | 21376 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.520 | 845291 | 21376 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.545 | 788637 | 21376 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.550 | 778683 | 21376 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.001 | 7789219 | 21632 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.148 | 8055600 | 21632 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.203 | 7481652 | 21632 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.212 | 5941846 | 21632 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.521 | 773700 | 21632 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.536 | 742321 | 21632 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.550 | 715716 | 21632 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `remote_queue:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.269 | 719993 | 45952 | `large_object` | 0 | 1144.02 | 0.186 | 0.023 |
| `fixed:balanced` | 0.349 | 443902 | 45952 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.415 | 333475 | 46080 | `cross_thread` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.445 | 321615 | 45952 | `deterministic_latency` | 0 | 1336.97 | 0.186 | 0.022 |
| `fixed:throughput_cache` | 0.457 | 307295 | 45952 | `throughput_cache` | 0 | 1336.97 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.477 | 180019 | 45952 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:fragmentation_stable` | 0.623 | 309635 | 53248 | `fragmentation_stable` | 0 | 1142.50 | 0.051 | 0.935 |

## `remote_queue:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.280 | 552600 | 46336 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:large_object` | 0.283 | 502462 | 46208 | `large_object` | 0 | 1144.02 | 0.186 | 0.023 |
| `fixed:throughput_cache` | 0.445 | 272436 | 46208 | `throughput_cache` | 0 | 1336.97 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.460 | 262194 | 46336 | `deterministic_latency` | 0 | 1336.97 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.464 | 239887 | 46336 | `cross_thread` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.477 | 159046 | 46208 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:fragmentation_stable` | 0.622 | 264659 | 53248 | `fragmentation_stable` | 0 | 1142.50 | 0.051 | 0.935 |

## `remote_queue:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.270 | 643058 | 46208 | `large_object` | 0 | 1180.49 | 0.187 | 0.024 |
| `fixed:balanced` | 0.324 | 509000 | 46464 | `balanced` | 0 | 1227.59 | 0.187 | 0.022 |
| `fixed:cross_thread` | 0.455 | 297505 | 46336 | `cross_thread` | 0 | 1227.59 | 0.187 | 0.022 |
| `fixed:deterministic_latency` | 0.471 | 308233 | 46464 | `deterministic_latency` | 0 | 1373.44 | 0.187 | 0.022 |
| `fixed:compact_rss` | 0.477 | 193187 | 46208 | `compact_rss` | 0 | 109.39 | 0.187 | 0.272 |
| `fixed:throughput_cache` | 0.505 | 278428 | 46464 | `throughput_cache` | 0 | 1373.44 | 0.187 | 0.022 |
| `fixed:fragmentation_stable` | 0.633 | 307532 | 52736 | `fragmentation_stable` | 0 | 1178.97 | 0.051 | 0.938 |

## `remote_queue:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.274 | 617587 | 46464 | `large_object` | 0 | 1192.64 | 0.183 | 0.024 |
| `fixed:balanced` | 0.308 | 502697 | 46336 | `balanced` | 0 | 1239.74 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.454 | 293070 | 46464 | `deterministic_latency` | 0 | 1385.59 | 0.183 | 0.022 |
| `fixed:compact_rss` | 0.482 | 172205 | 46464 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |
| `fixed:throughput_cache` | 0.491 | 261969 | 46592 | `throughput_cache` | 0 | 1385.59 | 0.183 | 0.022 |
| `fixed:cross_thread` | 0.583 | 189521 | 46336 | `cross_thread` | 0 | 1239.74 | 0.183 | 0.022 |
| `fixed:fragmentation_stable` | 0.648 | 266675 | 52992 | `fragmentation_stable` | 0 | 1191.12 | 0.052 | 0.921 |

## `remote_queue:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.273 | 692443 | 45952 | `large_object` | 0 | 1192.64 | 0.186 | 0.023 |
| `fixed:balanced` | 0.292 | 598197 | 45952 | `balanced` | 0 | 1239.74 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.368 | 314023 | 45952 | `cross_thread` | 0 | 1239.74 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.390 | 309379 | 45824 | `deterministic_latency` | 0 | 1385.59 | 0.186 | 0.022 |
| `fixed:throughput_cache` | 0.391 | 314086 | 45952 | `throughput_cache` | 0 | 1385.59 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.485 | 119221 | 46080 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:fragmentation_stable` | 0.582 | 272427 | 52992 | `fragmentation_stable` | 0 | 1191.12 | 0.051 | 0.924 |

## `remote_queue:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.277 | 720482 | 45952 | `large_object` | 0 | 1180.49 | 0.185 | 0.023 |
| `fixed:balanced` | 0.302 | 609303 | 45824 | `balanced` | 0 | 1227.59 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.449 | 326178 | 45824 | `deterministic_latency` | 0 | 1373.44 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.450 | 299651 | 45824 | `cross_thread` | 0 | 1227.59 | 0.185 | 0.022 |
| `fixed:throughput_cache` | 0.470 | 302827 | 45824 | `throughput_cache` | 0 | 1373.44 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.478 | 184360 | 45696 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:fragmentation_stable` | 0.650 | 287716 | 52864 | `fragmentation_stable` | 0 | 1178.97 | 0.051 | 0.922 |

## `remote_queue:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.273 | 697973 | 46336 | `large_object` | 0 | 1156.18 | 0.182 | 0.023 |
| `fixed:balanced` | 0.289 | 639149 | 46336 | `balanced` | 0 | 1203.28 | 0.182 | 0.022 |
| `fixed:cross_thread` | 0.397 | 303436 | 46336 | `cross_thread` | 0 | 1203.28 | 0.182 | 0.022 |
| `fixed:deterministic_latency` | 0.405 | 328292 | 46336 | `deterministic_latency` | 0 | 1349.13 | 0.182 | 0.022 |
| `fixed:throughput_cache` | 0.415 | 305605 | 46208 | `throughput_cache` | 0 | 1349.13 | 0.182 | 0.022 |
| `fixed:compact_rss` | 0.478 | 142289 | 46208 | `compact_rss` | 0 | 109.39 | 0.182 | 0.272 |
| `fixed:fragmentation_stable` | 0.637 | 242495 | 52736 | `fragmentation_stable` | 0 | 1154.66 | 0.052 | 0.903 |

## `remote_queue:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.273 | 689791 | 46336 | `large_object` | 0 | 1168.33 | 0.185 | 0.024 |
| `fixed:balanced` | 0.316 | 540043 | 46336 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.422 | 324160 | 46208 | `cross_thread` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:throughput_cache` | 0.461 | 310061 | 46336 | `throughput_cache` | 0 | 1361.28 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.466 | 304474 | 46336 | `deterministic_latency` | 0 | 1361.28 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.485 | 185362 | 46464 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:fragmentation_stable` | 0.638 | 298078 | 53504 | `fragmentation_stable` | 0 | 1166.81 | 0.052 | 0.931 |

## `remote_queue:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.277 | 664370 | 46208 | `balanced` | 0 | 1239.74 | 0.186 | 0.022 |
| `fixed:large_object` | 0.279 | 604266 | 46208 | `large_object` | 0 | 1192.64 | 0.186 | 0.023 |
| `fixed:cross_thread` | 0.389 | 302169 | 46208 | `cross_thread` | 0 | 1239.74 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.398 | 324984 | 46208 | `deterministic_latency` | 0 | 1385.59 | 0.186 | 0.022 |
| `fixed:throughput_cache` | 0.403 | 322479 | 46336 | `throughput_cache` | 0 | 1385.59 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.477 | 139927 | 46208 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:fragmentation_stable` | 0.638 | 236679 | 53376 | `fragmentation_stable` | 0 | 1191.12 | 0.051 | 0.933 |

## `remote_queue:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.269 | 703219 | 46464 | `large_object` | 0 | 1144.02 | 0.185 | 0.023 |
| `fixed:balanced` | 0.286 | 654905 | 46464 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.437 | 316492 | 46464 | `cross_thread` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:throughput_cache` | 0.450 | 329186 | 46464 | `throughput_cache` | 0 | 1336.97 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.459 | 322190 | 46592 | `deterministic_latency` | 0 | 1336.97 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.478 | 192141 | 46464 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:fragmentation_stable` | 0.632 | 313249 | 53248 | `fragmentation_stable` | 0 | 1142.50 | 0.052 | 0.925 |

## `remote_queue:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.272 | 685918 | 46080 | `large_object` | 0 | 1119.71 | 0.186 | 0.023 |
| `fixed:balanced` | 0.293 | 591798 | 46080 | `balanced` | 0 | 1166.81 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.401 | 278505 | 46080 | `cross_thread` | 0 | 1166.81 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.469 | 226324 | 45952 | `deterministic_latency` | 0 | 1312.66 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.481 | 131589 | 46080 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:throughput_cache` | 0.589 | 154469 | 46080 | `throughput_cache` | 0 | 1312.66 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.734 | 162907 | 52992 | `fragmentation_stable` | 0 | 1118.20 | 0.052 | 0.932 |

## `remote_queue:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.273 | 639195 | 46336 | `large_object` | 0 | 1156.18 | 0.183 | 0.023 |
| `fixed:balanced` | 0.277 | 636743 | 46208 | `balanced` | 0 | 1203.28 | 0.183 | 0.022 |
| `fixed:cross_thread` | 0.416 | 309989 | 46336 | `cross_thread` | 0 | 1203.28 | 0.183 | 0.022 |
| `fixed:throughput_cache` | 0.422 | 332250 | 46336 | `throughput_cache` | 0 | 1349.13 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.425 | 327592 | 46336 | `deterministic_latency` | 0 | 1349.13 | 0.183 | 0.022 |
| `fixed:compact_rss` | 0.433 | 189882 | 46328 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |
| `fixed:fragmentation_stable` | 0.819 | 170631 | 52864 | `fragmentation_stable` | 0 | 1154.66 | 0.052 | 0.912 |

## `remote_queue:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.269 | 732503 | 46464 | `large_object` | 0 | 1168.33 | 0.185 | 0.024 |
| `fixed:balanced` | 0.298 | 663314 | 46720 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.436 | 332962 | 46592 | `cross_thread` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.459 | 333241 | 46592 | `deterministic_latency` | 0 | 1361.28 | 0.185 | 0.022 |
| `fixed:throughput_cache` | 0.468 | 326160 | 46720 | `throughput_cache` | 0 | 1361.28 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.478 | 197568 | 46464 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:fragmentation_stable` | 0.650 | 304935 | 53376 | `fragmentation_stable` | 0 | 1166.81 | 0.052 | 0.928 |

## `remote_queue:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.276 | 529761 | 46848 | `large_object` | 0 | 1144.02 | 0.184 | 0.024 |
| `fixed:balanced` | 0.288 | 492442 | 46720 | `balanced` | 0 | 1191.12 | 0.184 | 0.022 |
| `fixed:cross_thread` | 0.391 | 259982 | 46592 | `cross_thread` | 0 | 1191.12 | 0.184 | 0.022 |
| `fixed:throughput_cache` | 0.404 | 277103 | 46720 | `throughput_cache` | 0 | 1336.97 | 0.184 | 0.022 |
| `fixed:deterministic_latency` | 0.416 | 262451 | 46720 | `deterministic_latency` | 0 | 1336.97 | 0.184 | 0.022 |
| `fixed:compact_rss` | 0.465 | 131635 | 46720 | `compact_rss` | 0 | 109.39 | 0.184 | 0.272 |
| `fixed:fragmentation_stable` | 0.818 | 126944 | 53504 | `fragmentation_stable` | 0 | 1142.50 | 0.051 | 0.935 |

## `remote_queue:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.272 | 651190 | 46464 | `large_object` | 0 | 1131.87 | 0.185 | 0.023 |
| `fixed:balanced` | 0.295 | 566192 | 46336 | `balanced` | 0 | 1178.97 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.405 | 322398 | 46336 | `cross_thread` | 0 | 1178.97 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.468 | 279286 | 46336 | `deterministic_latency` | 0 | 1324.82 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.481 | 172873 | 46464 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:throughput_cache` | 0.514 | 241576 | 46336 | `throughput_cache` | 0 | 1324.82 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.734 | 210067 | 53248 | `fragmentation_stable` | 0 | 1130.35 | 0.052 | 0.939 |

## `remote_queue:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.276 | 652403 | 46592 | `large_object` | 0 | 1144.02 | 0.183 | 0.024 |
| `fixed:balanced` | 0.282 | 640968 | 46464 | `balanced` | 0 | 1191.12 | 0.183 | 0.022 |
| `fixed:cross_thread` | 0.405 | 309473 | 46464 | `cross_thread` | 0 | 1191.12 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.442 | 288961 | 46336 | `deterministic_latency` | 0 | 1336.97 | 0.183 | 0.022 |
| `fixed:compact_rss` | 0.482 | 158980 | 46464 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |
| `fixed:throughput_cache` | 0.569 | 192686 | 46336 | `throughput_cache` | 0 | 1336.97 | 0.183 | 0.022 |
| `fixed:fragmentation_stable` | 0.725 | 199018 | 53120 | `fragmentation_stable` | 0 | 1142.50 | 0.052 | 0.917 |

## `remote_queue:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.276 | 576863 | 45952 | `large_object` | 0 | 1131.87 | 0.185 | 0.024 |
| `fixed:balanced` | 0.300 | 475406 | 45952 | `balanced` | 0 | 1178.97 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.396 | 240877 | 45952 | `cross_thread` | 0 | 1178.97 | 0.185 | 0.022 |
| `fixed:compact_rss` | 0.440 | 118372 | 45696 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:throughput_cache` | 0.462 | 198120 | 45952 | `throughput_cache` | 0 | 1324.82 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.482 | 179707 | 45824 | `deterministic_latency` | 0 | 1324.82 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.818 | 108178 | 52864 | `fragmentation_stable` | 0 | 1130.35 | 0.051 | 0.929 |

## `remote_queue:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.269 | 699368 | 46208 | `large_object` | 0 | 1144.02 | 0.186 | 0.023 |
| `fixed:balanced` | 0.306 | 565507 | 46208 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.421 | 326571 | 46208 | `cross_thread` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.458 | 314737 | 46336 | `deterministic_latency` | 0 | 1336.97 | 0.186 | 0.022 |
| `fixed:throughput_cache` | 0.466 | 303021 | 46208 | `throughput_cache` | 0 | 1336.97 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.477 | 185823 | 46208 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:fragmentation_stable` | 0.650 | 286820 | 53248 | `fragmentation_stable` | 0 | 1142.50 | 0.052 | 0.935 |

## `remote_queue:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.280 | 691685 | 46080 | `large_object` | 0 | 1119.71 | 0.186 | 0.024 |
| `fixed:balanced` | 0.286 | 721496 | 46208 | `balanced` | 0 | 1166.81 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.422 | 309341 | 46208 | `cross_thread` | 0 | 1166.81 | 0.186 | 0.022 |
| `fixed:throughput_cache` | 0.424 | 335206 | 46080 | `throughput_cache` | 0 | 1312.66 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.431 | 324811 | 46080 | `deterministic_latency` | 0 | 1312.66 | 0.186 | 0.022 |
| `fixed:compact_rss` | 0.477 | 162359 | 45824 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:fragmentation_stable` | 0.748 | 191799 | 53120 | `fragmentation_stable` | 0 | 1118.20 | 0.051 | 0.939 |

## `remote_queue:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.276 | 541840 | 46208 | `balanced` | 0 | 1203.28 | 0.184 | 0.022 |
| `fixed:large_object` | 0.288 | 488590 | 46208 | `large_object` | 0 | 1156.18 | 0.184 | 0.023 |
| `fixed:deterministic_latency` | 0.430 | 314847 | 46336 | `deterministic_latency` | 0 | 1349.13 | 0.184 | 0.022 |
| `fixed:throughput_cache` | 0.440 | 301089 | 46208 | `throughput_cache` | 0 | 1349.13 | 0.184 | 0.022 |
| `fixed:compact_rss` | 0.481 | 180769 | 46336 | `compact_rss` | 0 | 109.39 | 0.184 | 0.272 |
| `fixed:cross_thread` | 0.579 | 200431 | 46336 | `cross_thread` | 0 | 1203.28 | 0.184 | 0.022 |
| `fixed:fragmentation_stable` | 0.689 | 239940 | 52992 | `fragmentation_stable` | 0 | 1154.66 | 0.051 | 0.927 |

## `rss_peak_release:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.033 | 1286850 | 14208 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.076 | 1427143 | 14208 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.201 | 1151179 | 14208 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.221 | 510607 | 14208 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.240 | 342730 | 14208 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.352 | 110095 | 14208 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 49894 | 14208 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.038 | 1202279 | 14592 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.079 | 1481124 | 14592 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 1893600 | 14592 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.221 | 646894 | 14592 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.221 | 646857 | 14592 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.224 | 556985 | 14592 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 57402 | 14592 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.037 | 982318 | 15104 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.079 | 1194225 | 15104 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 1728062 | 15104 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.215 | 585860 | 15104 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.217 | 531285 | 15104 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.222 | 414847 | 15104 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 36171 | 15104 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.037 | 1348866 | 15488 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.079 | 1595982 | 15488 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 2151714 | 15488 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.220 | 650599 | 15488 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.224 | 622189 | 15488 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.226 | 591498 | 15488 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 58129 | 15488 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.038 | 1206633 | 15872 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.081 | 1372554 | 15872 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 2106566 | 15872 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.220 | 643147 | 15872 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.222 | 571283 | 15872 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.222 | 604354 | 15872 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 52075 | 15872 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.035 | 1248704 | 16256 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.078 | 1456362 | 16256 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 1701224 | 16256 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.221 | 557066 | 16256 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.224 | 552007 | 16256 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.224 | 550048 | 16256 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 53342 | 16256 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.039 | 1060191 | 16640 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.080 | 1384002 | 16640 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 1979000 | 16640 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.221 | 572224 | 16640 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.221 | 603540 | 16640 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.223 | 567663 | 16640 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 51035 | 16640 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.033 | 1311729 | 17024 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.076 | 1506502 | 17024 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.201 | 1173927 | 17024 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.216 | 621768 | 17024 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.222 | 552630 | 17024 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.222 | 543659 | 17024 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 51881 | 17024 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.084 | 1112792 | 17408 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 2138258 | 17408 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.221 | 584310 | 17408 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.221 | 622928 | 17408 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.223 | 588990 | 17408 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:large_object` | 0.244 | 857148 | 17536 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:compact_rss` | 0.450 | 52081 | 17408 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.080 | 1271328 | 17792 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 1697030 | 17792 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.220 | 599657 | 17792 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.222 | 562473 | 17792 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:large_object` | 0.237 | 1116029 | 17920 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:cross_thread` | 0.433 | 405857 | 17920 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 51154 | 17792 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.035 | 1221349 | 18304 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.079 | 1358640 | 18304 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 1712120 | 18304 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.218 | 570453 | 18304 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.224 | 499927 | 18304 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.224 | 495036 | 18304 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 46573 | 18304 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.198 | 1498617 | 18560 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:large_object` | 0.234 | 1209450 | 18688 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.278 | 1308446 | 18688 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:cross_thread` | 0.417 | 546404 | 18688 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.424 | 466401 | 18688 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.428 | 408445 | 18688 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.650 | 44264 | 18688 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.034 | 1248631 | 19072 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.076 | 1525661 | 19072 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.201 | 1227253 | 19072 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.216 | 625811 | 19072 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.221 | 569794 | 19072 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.226 | 506491 | 19072 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 53749 | 19072 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.198 | 2015337 | 19328 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.223 | 606545 | 19328 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:large_object` | 0.237 | 1242292 | 19456 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.279 | 1500440 | 19456 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:cross_thread` | 0.421 | 605451 | 19456 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.426 | 550044 | 19456 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.650 | 54883 | 19456 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.037 | 1204246 | 19840 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.080 | 1400091 | 19840 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 1940684 | 19840 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.221 | 631482 | 19840 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.223 | 578350 | 19840 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.226 | 504240 | 19840 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 53482 | 19840 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.198 | 1914271 | 20096 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:large_object` | 0.237 | 1122069 | 20224 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.281 | 1205281 | 20224 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:deterministic_latency` | 0.416 | 631205 | 20224 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.417 | 542529 | 20224 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.419 | 554689 | 20224 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.650 | 40870 | 20224 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.037 | 1101349 | 20480 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.080 | 1234900 | 20480 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 1659669 | 20480 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.218 | 598845 | 20480 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.230 | 430759 | 20480 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.230 | 403765 | 20480 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 48072 | 20480 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.198 | 1745764 | 20736 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:large_object` | 0.236 | 1221458 | 20864 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.278 | 1493968 | 20864 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:cross_thread` | 0.418 | 629076 | 20864 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.423 | 577878 | 20864 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.425 | 534915 | 20864 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.650 | 54117 | 20864 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.037 | 1303120 | 21120 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.080 | 1485708 | 21120 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.198 | 2034968 | 21120 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.220 | 621332 | 21120 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.224 | 577572 | 21120 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.224 | 571240 | 21120 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 54088 | 21120 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `rss_peak_release:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.036 | 1010682 | 21504 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.076 | 1317950 | 21504 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.200 | 1186978 | 21504 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.216 | 586429 | 21504 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.219 | 558476 | 21504 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.224 | 490119 | 21504 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.450 | 51506 | 21504 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |

## `throughput_churn:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.079 | 1983906 | 14080 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 8542015 | 14080 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.201 | 6726900 | 14080 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.227 | 737650 | 14080 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.228 | 697567 | 14080 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.236 | 568036 | 14080 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 61401 | 14080 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.200 | 7663323 | 14336 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.222 | 841182 | 14336 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.223 | 835643 | 14336 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.272 | 6151497 | 14464 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.400 | 7611490 | 14464 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.426 | 743069 | 14464 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.650 | 59863 | 14464 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 6482333 | 14848 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 7074242 | 14848 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 6630058 | 14848 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.232 | 627551 | 14848 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.240 | 509849 | 14848 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.274 | 282970 | 14848 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 61574 | 14848 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.072 | 5692709 | 15360 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 7427331 | 15360 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 6654318 | 15360 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.223 | 787586 | 15360 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.224 | 762451 | 15360 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.228 | 655017 | 15360 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 56814 | 15360 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.200 | 7366314 | 15616 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.271 | 7647704 | 15744 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.400 | 7529164 | 15744 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.424 | 805954 | 15744 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.425 | 770616 | 15744 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.427 | 723538 | 15744 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.650 | 61809 | 15744 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 6715461 | 16128 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 7712482 | 16128 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 6645746 | 16128 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.221 | 826165 | 16128 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.222 | 774250 | 16128 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.230 | 593336 | 16128 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 54701 | 16128 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 7903063 | 16512 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.201 | 6334359 | 16512 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.209 | 1895293 | 16512 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.226 | 778853 | 16512 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.230 | 671939 | 16512 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.266 | 322740 | 16512 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 63227 | 16512 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.200 | 7481168 | 16768 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.280 | 1835516 | 16896 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.400 | 6979413 | 16896 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.424 | 781447 | 16896 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.425 | 774679 | 16896 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.452 | 385916 | 16896 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.650 | 60124 | 16896 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.200 | 7344140 | 17280 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.201 | 6231311 | 17280 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.226 | 723105 | 17280 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.228 | 677019 | 17280 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.230 | 638444 | 17280 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.271 | 7882816 | 17408 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 58154 | 17280 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 8053522 | 17664 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 7850649 | 17664 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 7108969 | 17664 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.222 | 809235 | 17664 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.223 | 764849 | 17664 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.227 | 683103 | 17664 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 56242 | 17664 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.200 | 6921927 | 18048 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.223 | 779745 | 18048 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.232 | 571370 | 18048 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.271 | 7827997 | 18176 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.400 | 7680409 | 18176 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.422 | 789341 | 18176 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.650 | 55527 | 18176 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.200 | 7769600 | 18432 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 7554426 | 18432 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.224 | 818289 | 18432 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.271 | 294886 | 18432 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.271 | 7579477 | 18560 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.424 | 794880 | 18560 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 61464 | 18432 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 7831056 | 18816 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 7587072 | 18816 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 7319965 | 18816 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.222 | 813260 | 18816 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.223 | 773234 | 18816 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.223 | 770976 | 18816 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 55829 | 18816 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 6741356 | 19328 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 7744558 | 19328 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 7661062 | 19328 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.227 | 752509 | 19328 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.249 | 432265 | 19328 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.255 | 385084 | 19328 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 63237 | 19328 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.200 | 7521432 | 19712 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 7056460 | 19712 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.220 | 739919 | 19712 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.223 | 654546 | 19712 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.271 | 7634388 | 19840 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.420 | 741429 | 19840 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 47444 | 19712 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.200 | 7382071 | 19968 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.228 | 752088 | 19968 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.271 | 7721996 | 20096 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.400 | 8262549 | 20096 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.426 | 812117 | 20096 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.427 | 770322 | 20096 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.650 | 66096 | 20096 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 5372602 | 20352 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 6177167 | 20352 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.201 | 4891319 | 20352 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.219 | 806300 | 20352 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.219 | 798832 | 20352 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.235 | 455719 | 20352 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 49481 | 20352 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 6546227 | 20736 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 5686265 | 20736 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.201 | 5261181 | 20736 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.221 | 783070 | 20736 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.223 | 722134 | 20736 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.228 | 606673 | 20736 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 52398 | 20736 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 8248702 | 20992 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 8013800 | 20992 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.201 | 5977644 | 20992 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.226 | 811146 | 20992 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.229 | 714368 | 20992 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.234 | 629211 | 20992 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 65100 | 20992 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.071 | 6531164 | 21376 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.200 | 6529005 | 21376 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.200 | 5904730 | 21376 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.219 | 722260 | 21376 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.221 | 658108 | 21376 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.223 | 615026 | 21376 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.450 | 43947 | 21376 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

