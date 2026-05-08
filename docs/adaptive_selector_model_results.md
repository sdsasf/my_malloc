# Adaptive Selector Model Evaluation Results

Generated at: `2026-05-08 18:03:53 +0800`

These measurements compare fixed adaptive modes against the measured-cost `model` selector on the generated workload suite. When requested, the legacy `rule` selector is included only as a baseline. Lower score is better.

## Reproduction Command

```bash
python3 tools/evaluate_selector_model.py --bench-runner ./build/bench_runner --iters 20000 --slots 512 --threads 2 --seeds 20001 20002 20003 20004 20005 --mode-window 64 --mode-cooldown 0 --json-out models/selector_evaluation_results.json --markdown-out docs/adaptive_selector_model_results.md --include-rule-baseline
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
| 1 | `fixed:large_object` | 0.191 | 2051002 | 27736 | 0.0 | 18 |
| 2 | `fixed:compact_rss` | 0.225 | 912175 | 27537 | 0.0 | 11 |
| 3 | `selector:model` | 0.294 | 1641469 | 28039 | 1.0 | 1 |
| 4 | `fixed:fragmentation_stable` | 0.331 | 1764891 | 29385 | 0.0 | 2 |
| 5 | `fixed:balanced` | 0.333 | 1906252 | 29643 | 0.0 | 3 |
| 6 | `fixed:throughput_cache` | 0.369 | 322968 | 29872 | 0.0 | 0 |
| 7 | `fixed:deterministic_latency` | 0.379 | 327860 | 29864 | 0.0 | 0 |
| 8 | `selector:rule` | 0.381 | 1348905 | 28339 | 1.7 | 0 |
| 9 | `fixed:cross_thread` | 0.405 | 326789 | 29666 | 0.0 | 0 |

## Per-Workload Winners

| Workload | Best overall | Best fixed mode | Model selector |
|---|---|---|---|
| `adaptive_mix:seed20001` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20002` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20003` | `selector:model` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20004` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20005` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20001` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `fragmentation_drift:seed20002` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `fragmentation_drift:seed20003` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `fragmentation_drift:seed20004` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `fragmentation_drift:seed20005` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `large_burst:seed20001` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `large_burst:seed20002` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `large_burst:seed20003` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `large_burst:seed20004` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `large_burst:seed20005` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `latency_loop:seed20001` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `latency_loop:seed20002` | `fixed:balanced` | `fixed:balanced` | `selector:model` |
| `latency_loop:seed20003` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `latency_loop:seed20004` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `latency_loop:seed20005` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `remote_queue:seed20001` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `remote_queue:seed20002` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `remote_queue:seed20003` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `remote_queue:seed20004` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `remote_queue:seed20005` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `rss_peak_release:seed20001` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `rss_peak_release:seed20002` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `rss_peak_release:seed20003` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `rss_peak_release:seed20004` | `fixed:balanced` | `fixed:balanced` | `selector:model` |
| `rss_peak_release:seed20005` | `fixed:balanced` | `fixed:balanced` | `selector:model` |
| `throughput_churn:seed20001` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `throughput_churn:seed20002` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `throughput_churn:seed20003` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `throughput_churn:seed20004` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `throughput_churn:seed20005` | `fixed:large_object` | `fixed:large_object` | `selector:model` |

## `adaptive_mix:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.223 | 164349 | 37120 | `large_object` | 0 | 1071.10 | 0.004 | 0.755 |
| `selector:model` | 0.262 | 112582 | 37120 | `large_object` | 1 | 1071.10 | 0.004 | 0.755 |
| `fixed:compact_rss` | 0.328 | 81475 | 36776 | `compact_rss` | 0 | 109.39 | 0.005 | 0.983 |
| `fixed:fragmentation_stable` | 0.354 | 158854 | 39424 | `fragmentation_stable` | 0 | 2479.48 | 0.001 | 1.025 |
| `selector:rule` | 0.411 | 171353 | 39040 | `compact_rss` | 7 | 2576.71 | 0.004 | 0.744 |
| `fixed:balanced` | 0.662 | 208544 | 44012 | `balanced` | 0 | 5639.60 | 0.005 | 0.448 |
| `fixed:cross_thread` | 0.682 | 150636 | 44004 | `cross_thread` | 0 | 5639.60 | 0.005 | 0.448 |
| `fixed:throughput_cache` | 0.696 | 179660 | 44288 | `throughput_cache` | 0 | 6028.53 | 0.005 | 0.448 |
| `fixed:deterministic_latency` | 0.723 | 109912 | 44032 | `deterministic_latency` | 0 | 6028.53 | 0.005 | 0.448 |

## `adaptive_mix:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.258 | 181024 | 39168 | `large_object` | 0 | 1071.10 | 0.004 | 0.770 |
| `selector:model` | 0.266 | 175996 | 39168 | `large_object` | 1 | 1071.10 | 0.004 | 0.770 |
| `fixed:compact_rss` | 0.333 | 71167 | 38424 | `compact_rss` | 0 | 109.39 | 0.004 | 1.003 |
| `fixed:fragmentation_stable` | 0.353 | 179555 | 41600 | `fragmentation_stable` | 0 | 2528.09 | 0.001 | 1.026 |
| `selector:rule` | 0.556 | 109107 | 42240 | `compact_rss` | 7 | 3646.29 | 0.004 | 0.731 |
| `fixed:cross_thread` | 0.634 | 163202 | 45568 | `cross_thread` | 0 | 5688.21 | 0.004 | 0.462 |
| `fixed:balanced` | 0.644 | 139001 | 45564 | `balanced` | 0 | 5688.21 | 0.004 | 0.462 |
| `fixed:deterministic_latency` | 0.705 | 136653 | 46464 | `deterministic_latency` | 0 | 6271.62 | 0.004 | 0.462 |
| `fixed:throughput_cache` | 0.710 | 118213 | 46336 | `throughput_cache` | 0 | 6271.62 | 0.004 | 0.462 |

## `adaptive_mix:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.213 | 151888 | 38272 | `large_object` | 1 | 1071.10 | 0.004 | 0.773 |
| `fixed:large_object` | 0.226 | 118540 | 38272 | `large_object` | 0 | 1071.10 | 0.004 | 0.773 |
| `fixed:compact_rss` | 0.332 | 76464 | 38356 | `compact_rss` | 0 | 109.39 | 0.005 | 0.993 |
| `fixed:fragmentation_stable` | 0.348 | 129841 | 40704 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.038 |
| `selector:rule` | 0.404 | 120759 | 40064 | `compact_rss` | 7 | 2479.48 | 0.004 | 0.763 |
| `fixed:balanced` | 0.660 | 190232 | 45696 | `balanced` | 0 | 5590.98 | 0.005 | 0.456 |
| `fixed:cross_thread` | 0.696 | 124738 | 45824 | `cross_thread` | 0 | 5590.98 | 0.005 | 0.456 |
| `fixed:deterministic_latency` | 0.709 | 137828 | 45952 | `deterministic_latency` | 0 | 6077.15 | 0.005 | 0.456 |
| `fixed:throughput_cache` | 0.725 | 101183 | 45696 | `throughput_cache` | 0 | 6077.15 | 0.005 | 0.456 |

## `adaptive_mix:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.252 | 142602 | 37376 | `large_object` | 0 | 1071.10 | 0.004 | 0.751 |
| `selector:model` | 0.282 | 120634 | 37632 | `large_object` | 1 | 1071.10 | 0.004 | 0.751 |
| `fixed:fragmentation_stable` | 0.332 | 161809 | 39552 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 0.987 |
| `fixed:compact_rss` | 0.338 | 64586 | 37216 | `compact_rss` | 0 | 109.39 | 0.004 | 0.986 |
| `selector:rule` | 0.399 | 176815 | 38784 | `compact_rss` | 7 | 2333.63 | 0.004 | 0.744 |
| `fixed:balanced` | 0.646 | 215479 | 44160 | `balanced` | 0 | 5590.98 | 0.004 | 0.449 |
| `fixed:cross_thread` | 0.656 | 163893 | 44132 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.449 |
| `fixed:throughput_cache` | 0.699 | 163509 | 44672 | `throughput_cache` | 0 | 6077.15 | 0.004 | 0.449 |
| `fixed:deterministic_latency` | 0.699 | 157947 | 44672 | `deterministic_latency` | 0 | 6028.53 | 0.004 | 0.449 |

## `adaptive_mix:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.260 | 127312 | 37760 | `large_object` | 0 | 1071.10 | 0.004 | 0.754 |
| `selector:model` | 0.272 | 143590 | 38016 | `large_object` | 1 | 1071.10 | 0.004 | 0.754 |
| `fixed:compact_rss` | 0.335 | 68143 | 37452 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:fragmentation_stable` | 0.377 | 94214 | 40064 | `fragmentation_stable` | 0 | 2430.86 | 0.001 | 1.011 |
| `selector:rule` | 0.428 | 162713 | 39680 | `compact_rss` | 7 | 2430.86 | 0.004 | 0.743 |
| `fixed:balanced` | 0.639 | 189690 | 44500 | `balanced` | 0 | 5590.98 | 0.004 | 0.458 |
| `fixed:cross_thread` | 0.655 | 143502 | 44516 | `cross_thread` | 0 | 5590.98 | 0.004 | 0.458 |
| `fixed:throughput_cache` | 0.693 | 169217 | 45184 | `throughput_cache` | 0 | 6077.15 | 0.004 | 0.458 |
| `fixed:deterministic_latency` | 0.716 | 118629 | 45184 | `deterministic_latency` | 0 | 6077.15 | 0.004 | 0.458 |

## `fragmentation_drift:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.138 | 110916 | 47240 | `compact_rss` | 0 | 108.86 | 0.002 | 9.200 |
| `fixed:large_object` | 0.229 | 72457 | 47872 | `large_object` | 0 | 260.25 | 0.000 | 11.958 |
| `fixed:fragmentation_stable` | 0.368 | 102053 | 50176 | `fragmentation_stable` | 0 | 1837.08 | 0.001 | 10.947 |
| `selector:model` | 0.402 | 107490 | 50176 | `large_object` | 1 | 2219.80 | 0.000 | 11.738 |
| `selector:rule` | 0.471 | 63567 | 50176 | `large_object` | 1 | 2219.80 | 0.000 | 11.738 |
| `fixed:cross_thread` | 0.637 | 112720 | 54472 | `cross_thread` | 0 | 4449.81 | 0.002 | 9.143 |
| `fixed:balanced` | 0.647 | 105880 | 54544 | `balanced` | 0 | 4449.81 | 0.002 | 9.143 |
| `fixed:deterministic_latency` | 0.682 | 126488 | 55424 | `deterministic_latency` | 0 | 4721.97 | 0.002 | 9.143 |
| `fixed:throughput_cache` | 0.695 | 116542 | 55552 | `throughput_cache` | 0 | 4721.97 | 0.002 | 9.143 |

## `fragmentation_drift:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.160 | 111308 | 47872 | `compact_rss` | 0 | 108.86 | 0.002 | 9.721 |
| `fixed:large_object` | 0.188 | 133234 | 49024 | `large_object` | 0 | 355.51 | 0.000 | 12.415 |
| `selector:model` | 0.369 | 124181 | 50432 | `large_object` | 1 | 2260.62 | 0.000 | 12.281 |
| `fixed:fragmentation_stable` | 0.385 | 120141 | 51456 | `fragmentation_stable` | 0 | 1986.76 | 0.001 | 11.464 |
| `selector:rule` | 0.446 | 111833 | 51456 | `large_object` | 1 | 3022.67 | 0.000 | 12.006 |
| `fixed:cross_thread` | 0.606 | 130216 | 55128 | `cross_thread` | 0 | 4871.65 | 0.002 | 9.615 |
| `fixed:balanced` | 0.633 | 110605 | 55104 | `balanced` | 0 | 4871.65 | 0.002 | 9.615 |
| `fixed:throughput_cache` | 0.698 | 120621 | 56576 | `throughput_cache` | 0 | 5361.54 | 0.002 | 9.615 |
| `fixed:deterministic_latency` | 0.772 | 86845 | 56704 | `deterministic_latency` | 0 | 5361.54 | 0.002 | 9.615 |

## `fragmentation_drift:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.126 | 107696 | 47848 | `compact_rss` | 0 | 108.86 | 0.002 | 9.621 |
| `fixed:large_object` | 0.183 | 100435 | 48768 | `large_object` | 0 | 287.47 | 0.000 | 12.532 |
| `fixed:fragmentation_stable` | 0.358 | 99826 | 51200 | `fragmentation_stable` | 0 | 1809.86 | 0.001 | 11.579 |
| `selector:rule` | 0.365 | 106850 | 50816 | `large_object` | 1 | 2083.72 | 0.000 | 12.374 |
| `selector:model` | 0.444 | 63350 | 50688 | `large_object` | 1 | 2083.72 | 0.000 | 12.374 |
| `fixed:cross_thread` | 0.634 | 99039 | 55992 | `cross_thread` | 0 | 4749.18 | 0.002 | 9.568 |
| `fixed:balanced` | 0.654 | 86092 | 56004 | `balanced` | 0 | 4749.18 | 0.002 | 9.568 |
| `fixed:throughput_cache` | 0.686 | 104228 | 57088 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.568 |
| `fixed:deterministic_latency` | 0.709 | 91124 | 57216 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.568 |

## `fragmentation_drift:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.162 | 80768 | 47564 | `compact_rss` | 0 | 108.86 | 0.002 | 9.594 |
| `fixed:large_object` | 0.203 | 89807 | 48512 | `large_object` | 0 | 301.08 | 0.000 | 12.247 |
| `selector:model` | 0.366 | 115944 | 50304 | `large_object` | 1 | 2042.90 | 0.000 | 12.113 |
| `fixed:fragmentation_stable` | 0.384 | 88069 | 50944 | `fragmentation_stable` | 0 | 1823.47 | 0.001 | 11.255 |
| `selector:rule` | 0.406 | 74572 | 50176 | `large_object` | 1 | 2042.90 | 0.000 | 12.113 |
| `fixed:cross_thread` | 0.661 | 92078 | 55392 | `cross_thread` | 0 | 4653.93 | 0.002 | 9.538 |
| `fixed:deterministic_latency` | 0.702 | 92966 | 56064 | `deterministic_latency` | 0 | 4980.52 | 0.002 | 9.538 |
| `fixed:balanced` | 0.711 | 62509 | 55404 | `balanced` | 0 | 4653.93 | 0.002 | 9.538 |
| `fixed:throughput_cache` | 0.772 | 57403 | 56192 | `throughput_cache` | 0 | 4980.52 | 0.002 | 9.538 |

## `fragmentation_drift:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.129 | 133328 | 47700 | `compact_rss` | 0 | 108.86 | 0.002 | 10.042 |
| `fixed:large_object` | 0.180 | 126662 | 48512 | `large_object` | 0 | 301.08 | 0.000 | 12.547 |
| `fixed:fragmentation_stable` | 0.354 | 130765 | 50816 | `fragmentation_stable` | 0 | 1823.47 | 0.001 | 11.691 |
| `selector:rule` | 0.374 | 114134 | 50304 | `large_object` | 1 | 1934.03 | 0.000 | 12.360 |
| `selector:model` | 0.410 | 94643 | 50432 | `large_object` | 1 | 1934.03 | 0.000 | 12.360 |
| `fixed:cross_thread` | 0.630 | 132813 | 55292 | `cross_thread` | 0 | 4599.49 | 0.002 | 9.936 |
| `fixed:deterministic_latency` | 0.692 | 128195 | 56320 | `deterministic_latency` | 0 | 4926.09 | 0.002 | 9.936 |
| `fixed:throughput_cache` | 0.698 | 122534 | 56320 | `throughput_cache` | 0 | 4926.09 | 0.002 | 9.936 |
| `fixed:balanced` | 0.714 | 79165 | 55284 | `balanced` | 0 | 4599.49 | 0.002 | 9.936 |

## `large_burst:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.052 | 250355 | 14336 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `selector:model` | 0.100 | 271601 | 14336 | `large_object` | 1 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.123 | 226197 | 14336 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `selector:rule` | 0.172 | 260945 | 14336 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:balanced` | 0.178 | 332973 | 14336 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.181 | 288052 | 14336 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.184 | 248123 | 14336 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.189 | 196392 | 14336 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.214 | 51057 | 14336 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.051 | 284200 | 14848 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `selector:model` | 0.099 | 308168 | 14848 | `large_object` | 1 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.138 | 130326 | 14848 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `selector:rule` | 0.169 | 321960 | 14848 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:balanced` | 0.178 | 370809 | 14848 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.181 | 298088 | 14848 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.182 | 284878 | 14848 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.196 | 149761 | 14848 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.214 | 45499 | 14848 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.047 | 375171 | 15360 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `selector:model` | 0.100 | 307167 | 15360 | `large_object` | 1 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.117 | 355331 | 15360 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `selector:rule` | 0.170 | 312108 | 15360 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:cross_thread` | 0.182 | 300363 | 15360 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.186 | 254060 | 15360 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.192 | 198290 | 15360 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.194 | 186675 | 15360 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.214 | 56429 | 15360 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.052 | 231088 | 15872 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `selector:model` | 0.097 | 278258 | 15872 | `large_object` | 1 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.130 | 165838 | 15872 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `selector:rule` | 0.169 | 259636 | 15872 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:balanced` | 0.178 | 285687 | 15872 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.181 | 250305 | 15872 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.185 | 213847 | 15872 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.214 | 51726 | 15872 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |
| `fixed:cross_thread` | 0.220 | 88043 | 15872 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |

## `large_burst:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.049 | 297903 | 16256 | `large_object` | 0 | 138.70 | 0.000 | 1.002 |
| `selector:model` | 0.102 | 245331 | 16256 | `large_object` | 1 | 138.70 | 0.000 | 1.002 |
| `fixed:fragmentation_stable` | 0.120 | 280330 | 16256 | `fragmentation_stable` | 0 | 191.79 | 0.000 | 1.002 |
| `selector:rule` | 0.171 | 279941 | 16256 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:balanced` | 0.178 | 337425 | 16256 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.181 | 286484 | 16256 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.182 | 274124 | 16256 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.194 | 173899 | 16256 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.214 | 54198 | 16256 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `latency_loop:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.002 | 3580413 | 14592 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.140 | 1679330 | 14592 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 5668240 | 14592 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.180 | 3929600 | 14592 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.187 | 1458860 | 14592 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.229 | 3424526 | 14592 | `compact_rss` | 1 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.240 | 276638 | 14592 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.256 | 220549 | 14592 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.263 | 203053 | 14592 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.178 | 5855055 | 14976 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.252 | 319137 | 14976 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.386 | 5726678 | 15104 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.518 | 5320737 | 15104 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.564 | 5652335 | 15104 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.565 | 5024703 | 15104 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.614 | 458145 | 15104 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.614 | 4638938 | 15104 | `compact_rss` | 1 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.649 | 278635 | 15104 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 8016831 | 15488 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.132 | 8064581 | 15488 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.179 | 7179817 | 15488 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.182 | 5718039 | 15488 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.183 | 5179073 | 15488 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.229 | 6758244 | 15488 | `compact_rss` | 1 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.261 | 747118 | 15488 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.262 | 744531 | 15488 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.263 | 733357 | 15488 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.000 | 6013032 | 16000 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.132 | 6203408 | 16000 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.179 | 5884954 | 16000 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.190 | 2467417 | 16000 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.192 | 2300103 | 16000 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.230 | 4746657 | 16000 | `compact_rss` | 1 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.246 | 650134 | 16000 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.259 | 556302 | 16000 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.263 | 530249 | 16000 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.132 | 8259091 | 16384 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.179 | 7377479 | 16384 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.259 | 745730 | 16384 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.263 | 711111 | 16384 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.386 | 8228011 | 16512 | `large_object` | 0 | 70.68 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.564 | 8161192 | 16512 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.595 | 1673153 | 16512 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.616 | 5978188 | 16512 | `compact_rss` | 1 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.639 | 794882 | 16512 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `remote_queue:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.251 | 164050 | 46336 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.282 | 578716 | 46336 | `large_object` | 0 | 1168.33 | 0.184 | 0.023 |
| `fixed:balanced` | 0.288 | 600823 | 46336 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `selector:rule` | 0.291 | 555409 | 46336 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.333 | 305822 | 46208 | `deterministic_latency` | 0 | 1361.28 | 0.185 | 0.022 |
| `fixed:throughput_cache` | 0.337 | 321480 | 46336 | `throughput_cache` | 0 | 1361.28 | 0.185 | 0.022 |
| `selector:model` | 0.344 | 597545 | 46592 | `large_object` | 1 | 1168.33 | 0.184 | 0.023 |
| `fixed:cross_thread` | 0.364 | 178243 | 46336 | `cross_thread` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.722 | 218034 | 53120 | `fragmentation_stable` | 0 | 1166.81 | 0.053 | 0.919 |

## `remote_queue:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.252 | 167053 | 45952 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.285 | 607200 | 45952 | `large_object` | 0 | 1192.64 | 0.185 | 0.024 |
| `fixed:balanced` | 0.291 | 491036 | 45824 | `balanced` | 0 | 1239.74 | 0.185 | 0.022 |
| `selector:rule` | 0.302 | 662812 | 46208 | `balanced` | 0 | 1239.74 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.332 | 264179 | 45952 | `cross_thread` | 0 | 1239.74 | 0.185 | 0.022 |
| `selector:model` | 0.346 | 645474 | 46208 | `large_object` | 1 | 1192.64 | 0.185 | 0.024 |
| `fixed:deterministic_latency` | 0.350 | 270612 | 45952 | `deterministic_latency` | 0 | 1385.59 | 0.185 | 0.022 |
| `fixed:throughput_cache` | 0.355 | 280567 | 46080 | `throughput_cache` | 0 | 1385.59 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.743 | 180640 | 52864 | `fragmentation_stable` | 0 | 1191.12 | 0.053 | 0.913 |

## `remote_queue:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.251 | 123957 | 46336 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.282 | 609420 | 46336 | `large_object` | 0 | 1168.33 | 0.185 | 0.024 |
| `selector:rule` | 0.300 | 388852 | 46336 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:balanced` | 0.306 | 331326 | 46336 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.307 | 279401 | 46208 | `cross_thread` | 0 | 1215.43 | 0.185 | 0.022 |
| `selector:model` | 0.356 | 329053 | 46464 | `large_object` | 1 | 1168.33 | 0.185 | 0.024 |
| `fixed:deterministic_latency` | 0.357 | 172150 | 46208 | `deterministic_latency` | 0 | 1361.28 | 0.185 | 0.022 |
| `fixed:throughput_cache` | 0.375 | 150260 | 46336 | `throughput_cache` | 0 | 1361.28 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.750 | 125146 | 53248 | `fragmentation_stable` | 0 | 1166.81 | 0.051 | 0.931 |

## `remote_queue:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.244 | 169954 | 45952 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:balanced` | 0.281 | 535111 | 45952 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:large_object` | 0.289 | 539392 | 46208 | `large_object` | 0 | 1168.33 | 0.185 | 0.024 |
| `selector:rule` | 0.305 | 499529 | 46336 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.324 | 281785 | 46080 | `cross_thread` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.339 | 302475 | 46080 | `deterministic_latency` | 0 | 1361.28 | 0.185 | 0.022 |
| `fixed:throughput_cache` | 0.343 | 287225 | 46080 | `throughput_cache` | 0 | 1361.28 | 0.185 | 0.022 |
| `selector:model` | 0.347 | 432597 | 46208 | `large_object` | 1 | 1168.33 | 0.185 | 0.024 |
| `fixed:fragmentation_stable` | 0.713 | 244160 | 52992 | `fragmentation_stable` | 0 | 1166.81 | 0.052 | 0.917 |

## `remote_queue:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.244 | 140801 | 46336 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.281 | 568765 | 46464 | `large_object` | 0 | 1144.02 | 0.185 | 0.023 |
| `fixed:balanced` | 0.300 | 390882 | 46464 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `selector:rule` | 0.301 | 574224 | 46720 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.333 | 271143 | 46336 | `deterministic_latency` | 0 | 1336.97 | 0.185 | 0.022 |
| `selector:model` | 0.337 | 573627 | 46592 | `large_object` | 1 | 1144.02 | 0.185 | 0.023 |
| `fixed:throughput_cache` | 0.345 | 247730 | 46464 | `throughput_cache` | 0 | 1336.97 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.363 | 153658 | 46464 | `cross_thread` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.717 | 200092 | 53504 | `fragmentation_stable` | 0 | 1142.50 | 0.051 | 0.938 |

## `rss_peak_release:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.039 | 1026025 | 14336 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.073 | 1070719 | 14336 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `selector:model` | 0.148 | 681181 | 14336 | `large_object` | 1 | 3036.28 | 0.000 | 0.318 |
| `fixed:balanced` | 0.177 | 1175455 | 14336 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.180 | 494956 | 14336 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.200 | 115888 | 14336 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.202 | 107621 | 14336 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 32641 | 14336 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `selector:rule` | 0.432 | 34458 | 14336 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.075 | 1130629 | 14848 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `fixed:balanced` | 0.177 | 1865294 | 14848 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.184 | 548582 | 14848 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.184 | 543094 | 14848 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 49850 | 14848 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.426 | 1095104 | 14976 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `selector:model` | 0.534 | 925629 | 14976 | `large_object` | 1 | 3036.28 | 0.000 | 0.318 |
| `fixed:cross_thread` | 0.568 | 503648 | 14976 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `selector:rule` | 0.821 | 50614 | 14976 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.039 | 1032115 | 15360 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.075 | 912732 | 15360 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `selector:model` | 0.148 | 812851 | 15360 | `large_object` | 1 | 3036.28 | 0.000 | 0.318 |
| `fixed:balanced` | 0.177 | 1454262 | 15360 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.183 | 430902 | 15360 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.183 | 509345 | 15360 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.184 | 452969 | 15360 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.212 | 46298 | 15360 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `selector:rule` | 0.437 | 44879 | 15360 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.177 | 1755956 | 15872 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.184 | 519815 | 15872 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.184 | 476538 | 15872 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.204 | 49695 | 15872 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.425 | 1070668 | 16000 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.460 | 1129857 | 16000 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `selector:model` | 0.534 | 904348 | 16000 | `large_object` | 1 | 3036.28 | 0.000 | 0.318 |
| `fixed:cross_thread` | 0.567 | 538898 | 16000 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `selector:rule` | 0.823 | 43793 | 16000 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.177 | 1608264 | 16256 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.184 | 424440 | 16256 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.185 | 417739 | 16256 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.214 | 40708 | 16256 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.426 | 922169 | 16384 | `large_object` | 0 | 260.25 | 0.000 | 0.331 |
| `fixed:fragmentation_stable` | 0.460 | 1057015 | 16384 | `fragmentation_stable` | 0 | 1782.64 | 0.000 | 0.193 |
| `selector:model` | 0.535 | 716519 | 16384 | `large_object` | 1 | 3036.28 | 0.000 | 0.318 |
| `fixed:cross_thread` | 0.569 | 409828 | 16384 | `cross_thread` | 0 | 5375.15 | 0.000 | 0.048 |
| `selector:rule` | 0.820 | 41693 | 16384 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `throughput_churn:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.063 | 8089641 | 14080 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `selector:model` | 0.112 | 6207164 | 14080 | `large_object` | 1 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 7667877 | 14080 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.179 | 6409783 | 14080 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.185 | 764202 | 14080 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.185 | 706500 | 14080 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.186 | 685196 | 14080 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 63864 | 14080 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |
| `selector:rule` | 0.564 | 6191991 | 14208 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |

## `throughput_churn:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.179 | 1988779 | 14592 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.180 | 1816995 | 14592 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.184 | 571291 | 14592 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.184 | 551257 | 14592 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.185 | 449900 | 14592 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 40573 | 14592 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |
| `fixed:large_object` | 0.449 | 4794208 | 14720 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `selector:model` | 0.498 | 4630642 | 14720 | `large_object` | 1 | 137.78 | 0.000 | 0.001 |
| `selector:rule` | 0.564 | 4371278 | 14720 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |

## `throughput_churn:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.063 | 6416483 | 15104 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `selector:model` | 0.113 | 4116211 | 15104 | `large_object` | 1 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 6111441 | 15104 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 6000527 | 15104 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.179 | 4336251 | 15104 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.184 | 706061 | 15104 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.185 | 655464 | 15104 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.185 | 588743 | 15104 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 52926 | 15104 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.063 | 6432703 | 15616 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `selector:model` | 0.112 | 4829807 | 15616 | `large_object` | 1 | 137.78 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 5836810 | 15616 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.179 | 5001165 | 15616 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.179 | 4754154 | 15616 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.184 | 606802 | 15616 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.185 | 587791 | 15616 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.188 | 403772 | 15616 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 47753 | 15616 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.063 | 3742347 | 16128 | `large_object` | 0 | 137.78 | 0.000 | 0.001 |
| `selector:model` | 0.113 | 2881407 | 16128 | `large_object` | 1 | 137.78 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.178 | 5434386 | 16128 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.178 | 5153211 | 16128 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.179 | 3741503 | 16128 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.184 | 566808 | 16128 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.185 | 490010 | 16128 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.187 | 406568 | 16128 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.214 | 44087 | 16128 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

