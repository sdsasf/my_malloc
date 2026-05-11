# Adaptive Selector Model Evaluation Results

Generated at: `2026-05-11 22:01:52 +0800`

These measurements compare fixed adaptive modes against the measured-cost `model` selector on the generated workload suite. When requested, the legacy `rule` selector is included only as a baseline. Lower score is better.

## Reproduction Command

```bash
python3 tools/evaluate_selector_model.py --bench-runner ./build/bench_runner --iters 20000 --slots 512 --threads 2 --seeds 20001 20002 20003 20004 20005 --mode-window 64 --mode-cooldown 0 --json-out models/selector_evaluation_results.json --markdown-out docs/adaptive_selector_model_results.md --include-rule-baseline
```

## Score Definition

Score is normalized per workload across all compared cases. The metric weights are loaded from the trained objective model summary, so the report uses the same learned objective family as training:

| Metric | Weight | Direction |
|---|---:|---|
| `fragmentation_estimate` | 0.32 | lower is better |
| `mapped_live_ratio` | 0.13 | lower is better |
| `mode_switches` | 0.01 | lower is better |
| `ms` | 0.17 | lower is better |
| `peak_rss_kb` | 0.21 | lower is better |
| `slow_path_ratio` | 0.15 | lower is better |
| `validation_errors` | 0.01 | lower is better |

## Overall Ranking

| Rank | Case | Mean score | Mean ops/sec | Mean peak RSS KB | Mean switches | Best workload count |
|---:|---|---:|---:|---:|---:|---:|
| 1 | `selector:model` | 0.238 | 1826740 | 27941 | 8.1 | 7 |
| 2 | `fixed:fragmentation_stable` | 0.252 | 2367375 | 29114 | 0.0 | 9 |
| 3 | `fixed:balanced` | 0.252 | 2226204 | 29448 | 0.0 | 2 |
| 4 | `fixed:deterministic_latency` | 0.276 | 2472478 | 29718 | 0.0 | 2 |
| 5 | `fixed:throughput_cache` | 0.298 | 2551533 | 30021 | 0.0 | 3 |
| 6 | `fixed:large_object` | 0.299 | 1095295 | 27165 | 0.0 | 9 |
| 7 | `fixed:cross_thread` | 0.308 | 2515009 | 29923 | 0.0 | 1 |
| 8 | `fixed:compact_rss` | 0.345 | 1125499 | 27361 | 0.0 | 2 |
| 9 | `selector:rule` | 0.366 | 1619620 | 29593 | 1.6 | 0 |

## Per-Workload Winners

| Workload | Best overall | Best fixed mode | Model selector |
|---|---|---|---|
| `adaptive_mix:seed20001` | `selector:model` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20002` | `selector:model` | `fixed:fragmentation_stable` | `selector:model` |
| `adaptive_mix:seed20003` | `selector:model` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20004` | `selector:model` | `fixed:fragmentation_stable` | `selector:model` |
| `adaptive_mix:seed20005` | `selector:model` | `fixed:fragmentation_stable` | `selector:model` |
| `fragmentation_drift:seed20001` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20002` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20003` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20004` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20005` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `large_burst:seed20001` | `fixed:cross_thread` | `fixed:cross_thread` | `selector:model` |
| `large_burst:seed20002` | `selector:model` | `fixed:fragmentation_stable` | `selector:model` |
| `large_burst:seed20003` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `selector:model` |
| `large_burst:seed20004` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `large_burst:seed20005` | `selector:model` | `fixed:cross_thread` | `selector:model` |
| `latency_loop:seed20001` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `latency_loop:seed20002` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `latency_loop:seed20003` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `latency_loop:seed20004` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `latency_loop:seed20005` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `remote_queue:seed20001` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `remote_queue:seed20002` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `remote_queue:seed20003` | `fixed:balanced` | `fixed:balanced` | `selector:model` |
| `remote_queue:seed20004` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `remote_queue:seed20005` | `fixed:balanced` | `fixed:balanced` | `selector:model` |
| `rss_peak_release:seed20001` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `rss_peak_release:seed20002` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `rss_peak_release:seed20003` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `rss_peak_release:seed20004` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `rss_peak_release:seed20005` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `throughput_churn:seed20001` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `throughput_churn:seed20002` | `fixed:throughput_cache` | `fixed:throughput_cache` | `selector:model` |
| `throughput_churn:seed20003` | `fixed:throughput_cache` | `fixed:throughput_cache` | `selector:model` |
| `throughput_churn:seed20004` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `selector:model` |
| `throughput_churn:seed20005` | `fixed:throughput_cache` | `fixed:throughput_cache` | `selector:model` |

## `adaptive_mix:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.213 | 160399 | 38212 | `throughput_cache` | 9 | 3062.88 | 0.004 | 0.744 |
| `fixed:large_object` | 0.313 | 72336 | 35692 | `large_object` | 0 | 62.29 | 0.004 | 1.267 |
| `fixed:fragmentation_stable` | 0.560 | 218530 | 42720 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.451 |
| `fixed:compact_rss` | 0.565 | 86952 | 36592 | `compact_rss` | 0 | 109.39 | 0.005 | 0.983 |
| `fixed:balanced` | 0.619 | 214104 | 43860 | `balanced` | 0 | 5639.60 | 0.005 | 0.448 |
| `fixed:deterministic_latency` | 0.638 | 234396 | 44160 | `deterministic_latency` | 0 | 6806.41 | 0.005 | 0.448 |
| `selector:rule` | 0.649 | 180633 | 44128 | `compact_rss` | 7 | 5202.04 | 0.005 | 0.473 |
| `fixed:cross_thread` | 0.649 | 225773 | 44160 | `cross_thread` | 0 | 7243.96 | 0.005 | 0.451 |
| `fixed:throughput_cache` | 0.663 | 222294 | 44288 | `throughput_cache` | 0 | 7754.44 | 0.005 | 0.456 |

## `adaptive_mix:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.185 | 172800 | 39456 | `throughput_cache` | 11 | 2625.33 | 0.004 | 0.763 |
| `fixed:fragmentation_stable` | 0.239 | 210767 | 44908 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.467 |
| `fixed:balanced` | 0.280 | 210846 | 45396 | `balanced` | 0 | 5688.21 | 0.004 | 0.462 |
| `fixed:compact_rss` | 0.283 | 84316 | 38596 | `compact_rss` | 0 | 109.39 | 0.004 | 1.004 |
| `selector:rule` | 0.294 | 203774 | 45624 | `compact_rss` | 7 | 5202.04 | 0.004 | 0.488 |
| `fixed:large_object` | 0.313 | 80156 | 37192 | `large_object` | 0 | 62.29 | 0.004 | 1.283 |
| `fixed:deterministic_latency` | 0.321 | 214550 | 46336 | `deterministic_latency` | 0 | 7049.50 | 0.004 | 0.462 |
| `fixed:cross_thread` | 0.327 | 215017 | 46208 | `cross_thread` | 0 | 7535.67 | 0.004 | 0.467 |
| `fixed:throughput_cache` | 0.346 | 202044 | 46336 | `throughput_cache` | 0 | 8058.30 | 0.004 | 0.471 |

## `adaptive_mix:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.191 | 182060 | 39252 | `throughput_cache` | 15 | 2819.80 | 0.004 | 0.764 |
| `fixed:large_object` | 0.313 | 82153 | 36888 | `large_object` | 0 | 62.29 | 0.004 | 1.285 |
| `fixed:fragmentation_stable` | 0.563 | 202203 | 44212 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.460 |
| `fixed:compact_rss` | 0.600 | 86758 | 38212 | `compact_rss` | 0 | 109.39 | 0.005 | 0.994 |
| `fixed:balanced` | 0.621 | 213274 | 45696 | `balanced` | 0 | 5590.98 | 0.005 | 0.456 |
| `selector:rule` | 0.633 | 204042 | 45824 | `compact_rss` | 7 | 5202.04 | 0.005 | 0.481 |
| `fixed:deterministic_latency` | 0.646 | 214455 | 45824 | `deterministic_latency` | 0 | 6903.64 | 0.005 | 0.456 |
| `fixed:cross_thread` | 0.649 | 215448 | 45696 | `cross_thread` | 0 | 7292.58 | 0.005 | 0.459 |
| `fixed:throughput_cache` | 0.656 | 220621 | 45696 | `throughput_cache` | 0 | 7790.91 | 0.005 | 0.463 |

## `adaptive_mix:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.197 | 169741 | 38040 | `throughput_cache` | 17 | 2576.71 | 0.004 | 0.745 |
| `fixed:fragmentation_stable` | 0.237 | 227540 | 43084 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.453 |
| `fixed:compact_rss` | 0.272 | 86782 | 37100 | `compact_rss` | 0 | 109.39 | 0.004 | 0.986 |
| `fixed:balanced` | 0.287 | 220789 | 43904 | `balanced` | 0 | 5590.98 | 0.004 | 0.449 |
| `selector:rule` | 0.304 | 204141 | 44160 | `compact_rss` | 7 | 5202.04 | 0.004 | 0.474 |
| `fixed:large_object` | 0.313 | 77980 | 35664 | `large_object` | 0 | 62.29 | 0.004 | 1.263 |
| `fixed:deterministic_latency` | 0.320 | 232391 | 44672 | `deterministic_latency` | 0 | 6757.79 | 0.004 | 0.449 |
| `fixed:cross_thread` | 0.324 | 236914 | 44544 | `cross_thread` | 0 | 7219.66 | 0.004 | 0.453 |
| `fixed:throughput_cache` | 0.349 | 211736 | 44800 | `throughput_cache` | 0 | 7717.98 | 0.004 | 0.457 |

## `adaptive_mix:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.182 | 196034 | 38756 | `throughput_cache` | 11 | 2673.95 | 0.004 | 0.744 |
| `fixed:fragmentation_stable` | 0.237 | 212794 | 43416 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.461 |
| `fixed:compact_rss` | 0.283 | 86989 | 37588 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:balanced` | 0.284 | 222230 | 44372 | `balanced` | 0 | 5590.98 | 0.004 | 0.458 |
| `selector:rule` | 0.295 | 228057 | 44640 | `compact_rss` | 7 | 5202.04 | 0.004 | 0.483 |
| `fixed:large_object` | 0.313 | 83066 | 36344 | `large_object` | 0 | 62.29 | 0.004 | 1.266 |
| `fixed:deterministic_latency` | 0.324 | 211793 | 44928 | `deterministic_latency` | 0 | 6903.64 | 0.004 | 0.458 |
| `fixed:cross_thread` | 0.339 | 210429 | 45184 | `cross_thread` | 0 | 7353.35 | 0.004 | 0.461 |
| `fixed:throughput_cache` | 0.340 | 212325 | 44928 | `throughput_cache` | 0 | 7790.91 | 0.004 | 0.466 |

## `fragmentation_drift:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.244 | 122118 | 46776 | `large_object` | 0 | 56.13 | 0.000 | 11.958 |
| `selector:model` | 0.369 | 121790 | 49444 | `deterministic_latency` | 2 | 2082.02 | 0.000 | 11.732 |
| `fixed:compact_rss` | 0.501 | 113698 | 47148 | `compact_rss` | 0 | 108.86 | 0.002 | 9.200 |
| `fixed:balanced` | 0.644 | 132626 | 54204 | `balanced` | 0 | 4449.81 | 0.002 | 9.143 |
| `selector:rule` | 0.650 | 132935 | 54556 | `balanced` | 0 | 4449.81 | 0.002 | 9.143 |
| `fixed:throughput_cache` | 0.658 | 136233 | 55424 | `throughput_cache` | 0 | 4721.97 | 0.002 | 9.143 |
| `fixed:fragmentation_stable` | 0.684 | 119427 | 52936 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.143 |
| `fixed:cross_thread` | 0.724 | 126303 | 55424 | `cross_thread` | 0 | 4721.97 | 0.002 | 9.143 |
| `fixed:deterministic_latency` | 0.808 | 115673 | 55424 | `deterministic_latency` | 0 | 4721.97 | 0.002 | 9.143 |

## `fragmentation_drift:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.279 | 121958 | 47396 | `large_object` | 0 | 56.13 | 0.000 | 12.415 |
| `fixed:compact_rss` | 0.407 | 126661 | 47808 | `compact_rss` | 0 | 108.86 | 0.002 | 9.725 |
| `selector:model` | 0.425 | 119530 | 49792 | `deterministic_latency` | 2 | 2109.24 | 0.000 | 12.275 |
| `fixed:fragmentation_stable` | 0.608 | 128482 | 54192 | `fragmentation_stable` | 0 | 3619.72 | 0.002 | 9.615 |
| `selector:rule` | 0.635 | 130812 | 55268 | `balanced` | 0 | 4871.65 | 0.002 | 9.615 |
| `fixed:balanced` | 0.651 | 129471 | 55264 | `balanced` | 0 | 4871.65 | 0.002 | 9.615 |
| `fixed:throughput_cache` | 0.655 | 132362 | 56448 | `throughput_cache` | 0 | 5361.54 | 0.002 | 9.615 |
| `fixed:deterministic_latency` | 0.721 | 127212 | 56576 | `deterministic_latency` | 0 | 5361.54 | 0.002 | 9.615 |
| `fixed:cross_thread` | 0.789 | 121666 | 56320 | `cross_thread` | 0 | 5361.54 | 0.002 | 9.615 |

## `fragmentation_drift:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.210 | 119423 | 47540 | `large_object` | 0 | 56.13 | 0.000 | 12.532 |
| `selector:model` | 0.361 | 114970 | 49808 | `deterministic_latency` | 2 | 1918.72 | 0.000 | 12.368 |
| `fixed:compact_rss` | 0.500 | 110052 | 47868 | `compact_rss` | 0 | 108.86 | 0.002 | 9.636 |
| `fixed:fragmentation_stable` | 0.610 | 120819 | 54352 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.568 |
| `fixed:balanced` | 0.624 | 126089 | 55860 | `balanced` | 0 | 4749.18 | 0.002 | 9.568 |
| `fixed:throughput_cache` | 0.662 | 125552 | 56960 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.568 |
| `fixed:deterministic_latency` | 0.693 | 122020 | 56832 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.568 |
| `fixed:cross_thread` | 0.703 | 121050 | 56832 | `cross_thread` | 0 | 5130.21 | 0.002 | 9.568 |
| `selector:rule` | 0.731 | 115823 | 56120 | `balanced` | 0 | 4749.18 | 0.002 | 9.568 |

## `fragmentation_drift:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.195 | 125509 | 47272 | `large_object` | 0 | 56.13 | 0.000 | 12.247 |
| `selector:model` | 0.329 | 122893 | 49480 | `deterministic_latency` | 2 | 1877.90 | 0.000 | 12.108 |
| `fixed:compact_rss` | 0.492 | 115541 | 47576 | `compact_rss` | 0 | 108.86 | 0.002 | 9.598 |
| `fixed:fragmentation_stable` | 0.628 | 124268 | 53856 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.538 |
| `fixed:cross_thread` | 0.658 | 130540 | 56192 | `cross_thread` | 0 | 4980.52 | 0.002 | 9.538 |
| `fixed:balanced` | 0.664 | 126809 | 55296 | `balanced` | 0 | 4653.93 | 0.002 | 9.538 |
| `selector:rule` | 0.674 | 126347 | 55512 | `balanced` | 0 | 4653.93 | 0.002 | 9.538 |
| `fixed:throughput_cache` | 0.691 | 126788 | 56064 | `throughput_cache` | 0 | 4980.52 | 0.002 | 9.538 |
| `fixed:deterministic_latency` | 0.825 | 114935 | 56192 | `deterministic_latency` | 0 | 4980.52 | 0.002 | 9.538 |

## `fragmentation_drift:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.275 | 98600 | 47264 | `large_object` | 0 | 56.13 | 0.000 | 12.547 |
| `selector:model` | 0.327 | 102160 | 49628 | `deterministic_latency` | 2 | 1755.43 | 0.000 | 12.355 |
| `fixed:compact_rss` | 0.465 | 98528 | 47576 | `compact_rss` | 0 | 108.86 | 0.002 | 10.042 |
| `fixed:fragmentation_stable` | 0.575 | 106572 | 53464 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.936 |
| `fixed:throughput_cache` | 0.658 | 108099 | 56320 | `throughput_cache` | 0 | 4926.09 | 0.002 | 9.936 |
| `fixed:balanced` | 0.658 | 105225 | 55132 | `balanced` | 0 | 4599.49 | 0.002 | 9.936 |
| `fixed:deterministic_latency` | 0.673 | 106848 | 56320 | `deterministic_latency` | 0 | 4926.09 | 0.002 | 9.936 |
| `fixed:cross_thread` | 0.690 | 105519 | 56320 | `cross_thread` | 0 | 4926.09 | 0.002 | 9.936 |
| `selector:rule` | 0.796 | 96092 | 55408 | `balanced` | 0 | 4599.49 | 0.002 | 9.936 |

## `large_burst:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.130 | 411957 | 13952 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.131 | 397015 | 13952 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.134 | 366998 | 13952 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.136 | 338399 | 13952 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.139 | 311662 | 13952 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `selector:model` | 0.332 | 330944 | 14080 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `selector:rule` | 0.336 | 300072 | 14080 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:compact_rss` | 0.345 | 59085 | 13952 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |
| `fixed:large_object` | 0.521 | 58743 | 14080 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |

## `large_burst:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.123 | 348319 | 14592 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `selector:rule` | 0.125 | 329314 | 14592 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:fragmentation_stable` | 0.130 | 406785 | 14592 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.130 | 402272 | 14592 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.134 | 359587 | 14592 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.136 | 331649 | 14592 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.145 | 265211 | 14592 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.303 | 61167 | 14592 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.346 | 57969 | 14592 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.130 | 408166 | 14976 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.133 | 365183 | 14976 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.135 | 344971 | 14976 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `selector:rule` | 0.329 | 362279 | 15104 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `selector:model` | 0.336 | 299980 | 15104 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:cross_thread` | 0.338 | 400469 | 15104 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.342 | 349266 | 15104 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:compact_rss` | 0.343 | 58164 | 14976 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |
| `fixed:large_object` | 0.521 | 57249 | 15104 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |

## `large_burst:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.130 | 386852 | 15488 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.131 | 373802 | 15488 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.131 | 372083 | 15488 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.132 | 366979 | 15488 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.135 | 328391 | 15488 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.302 | 58007 | 15488 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `selector:rule` | 0.327 | 365550 | 15616 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `selector:model` | 0.331 | 319807 | 15616 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:compact_rss` | 0.346 | 54733 | 15488 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.118 | 308080 | 16000 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `selector:rule` | 0.124 | 261140 | 16000 | `large_object` | 1 | 193.50 | 0.000 | 1.001 |
| `fixed:cross_thread` | 0.130 | 308350 | 16000 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.130 | 306076 | 16000 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.130 | 305528 | 16000 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.130 | 305471 | 16000 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.131 | 302303 | 16000 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.313 | 48954 | 16000 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.343 | 49819 | 16000 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `latency_loop:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.116 | 8012429 | 14208 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.155 | 7647128 | 14208 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.173 | 7190325 | 14208 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.195 | 6701274 | 14208 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.212 | 8251188 | 14336 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.338 | 8400411 | 14336 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.444 | 5918970 | 14336 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.458 | 5706441 | 14336 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.505 | 5067875 | 14336 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.006 | 8232551 | 14720 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.104 | 8469199 | 14720 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.130 | 8475449 | 14720 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.131 | 8435734 | 14720 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.140 | 8037711 | 14720 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.180 | 6727535 | 14720 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.198 | 6243402 | 14720 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.211 | 5953551 | 14720 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.298 | 4510689 | 14720 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.008 | 7533943 | 15232 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.118 | 7332652 | 15232 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.130 | 7809265 | 15232 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.139 | 7513672 | 15232 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.162 | 6858693 | 15232 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.169 | 6678363 | 15232 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.222 | 5580605 | 15232 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.237 | 5333924 | 15232 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.298 | 4514856 | 15232 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.120 | 7944176 | 15744 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.130 | 8253426 | 15744 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.136 | 8150281 | 15744 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.136 | 8137195 | 15744 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.155 | 6036539 | 15744 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:balanced` | 0.174 | 7471346 | 15744 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.213 | 6899878 | 15744 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.288 | 6014615 | 15744 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.298 | 5912740 | 15744 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.054 | 5648446 | 16256 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.130 | 6613172 | 16256 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.156 | 6103428 | 16256 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.162 | 6007867 | 16256 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.204 | 5002751 | 16256 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.216 | 5196303 | 16256 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.274 | 4533535 | 16256 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.278 | 4490692 | 16256 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.298 | 4308594 | 16256 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `remote_queue:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.294 | 168125 | 46208 | `large_object` | 0 | 62.29 | 0.184 | 0.273 |
| `fixed:balanced` | 0.352 | 657754 | 46208 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.368 | 552788 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `selector:rule` | 0.379 | 626324 | 46464 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.395 | 681011 | 46464 | `deterministic_latency` | 0 | 2212.08 | 0.185 | 0.022 |
| `selector:model` | 0.459 | 626741 | 47104 | `deterministic_latency` | 34 | 1422.05 | 0.185 | 0.023 |
| `fixed:cross_thread` | 0.523 | 593868 | 47616 | `cross_thread` | 0 | 2710.41 | 0.185 | 0.024 |
| `fixed:compact_rss` | 0.646 | 152208 | 46336 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:throughput_cache` | 0.659 | 800515 | 48384 | `throughput_cache` | 0 | 6466.09 | 0.185 | 0.025 |

## `remote_queue:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.035 | 578858 | 45568 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.066 | 656848 | 45952 | `balanced` | 0 | 1239.74 | 0.185 | 0.022 |
| `selector:rule` | 0.071 | 606650 | 45952 | `balanced` | 0 | 1239.74 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.086 | 690399 | 45952 | `deterministic_latency` | 0 | 2297.16 | 0.185 | 0.023 |
| `selector:model` | 0.151 | 638232 | 46720 | `deterministic_latency` | 34 | 1373.44 | 0.185 | 0.023 |
| `fixed:cross_thread` | 0.215 | 688002 | 47360 | `cross_thread` | 0 | 2722.56 | 0.185 | 0.024 |
| `fixed:compact_rss` | 0.327 | 178097 | 45824 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:throughput_cache` | 0.339 | 786158 | 48000 | `throughput_cache` | 0 | 6417.47 | 0.185 | 0.025 |
| `fixed:large_object` | 0.346 | 171013 | 45952 | `large_object` | 0 | 62.29 | 0.185 | 0.273 |

## `remote_queue:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.027 | 660086 | 46208 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.028 | 603842 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.054 | 614457 | 46208 | `deterministic_latency` | 0 | 2272.86 | 0.185 | 0.023 |
| `selector:model` | 0.093 | 678947 | 46720 | `deterministic_latency` | 42 | 1373.44 | 0.185 | 0.023 |
| `selector:rule` | 0.133 | 638798 | 47232 | `balanced` | 2 | 1470.67 | 0.185 | 0.023 |
| `fixed:cross_thread` | 0.203 | 689840 | 47744 | `cross_thread` | 0 | 2746.87 | 0.185 | 0.024 |
| `fixed:large_object` | 0.306 | 179513 | 46208 | `large_object` | 0 | 62.29 | 0.185 | 0.273 |
| `fixed:compact_rss` | 0.314 | 173344 | 46208 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:throughput_cache` | 0.340 | 712187 | 48384 | `throughput_cache` | 0 | 6526.86 | 0.185 | 0.026 |

## `remote_queue:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.048 | 537874 | 45952 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.049 | 572919 | 45952 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `selector:rule` | 0.067 | 612109 | 46208 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.085 | 643377 | 46208 | `deterministic_latency` | 0 | 2224.24 | 0.185 | 0.022 |
| `selector:model` | 0.109 | 651755 | 46464 | `deterministic_latency` | 32 | 1604.37 | 0.185 | 0.023 |
| `fixed:cross_thread` | 0.247 | 568522 | 47872 | `cross_thread` | 0 | 2807.64 | 0.185 | 0.025 |
| `fixed:compact_rss` | 0.298 | 169322 | 45824 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.335 | 157384 | 46080 | `large_object` | 0 | 62.29 | 0.185 | 0.274 |
| `fixed:throughput_cache` | 0.339 | 773383 | 48256 | `throughput_cache` | 0 | 6526.86 | 0.185 | 0.025 |

## `remote_queue:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.041 | 563484 | 46336 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.047 | 469687 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `selector:rule` | 0.064 | 501894 | 46592 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.093 | 575166 | 46848 | `deterministic_latency` | 0 | 2175.62 | 0.185 | 0.022 |
| `selector:model` | 0.126 | 446086 | 47104 | `deterministic_latency` | 48 | 1494.98 | 0.185 | 0.023 |
| `fixed:cross_thread` | 0.215 | 425913 | 48256 | `cross_thread` | 0 | 2746.87 | 0.185 | 0.024 |
| `fixed:compact_rss` | 0.306 | 148030 | 46208 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.330 | 142488 | 46464 | `large_object` | 0 | 62.29 | 0.185 | 0.273 |
| `fixed:throughput_cache` | 0.340 | 684740 | 49408 | `throughput_cache` | 0 | 6478.24 | 0.185 | 0.026 |

## `rss_peak_release:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.086 | 1711185 | 14080 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `selector:model` | 0.117 | 898388 | 14080 | `throughput_cache` | 3 | 2912.11 | 0.000 | 0.318 |
| `fixed:balanced` | 0.129 | 1869688 | 14080 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.130 | 2000595 | 14080 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.130 | 1890309 | 14080 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.130 | 1845608 | 14080 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.279 | 52659 | 14080 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.313 | 50970 | 14080 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |
| `selector:rule` | 0.419 | 52817 | 14080 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.085 | 1915960 | 14592 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.129 | 1720015 | 14592 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.130 | 1921452 | 14592 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.130 | 1919605 | 14592 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.285 | 53167 | 14592 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `selector:model` | 0.324 | 949232 | 14720 | `throughput_cache` | 3 | 2912.11 | 0.000 | 0.318 |
| `fixed:cross_thread` | 0.338 | 1890079 | 14720 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:large_object` | 0.515 | 55190 | 14720 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |
| `selector:rule` | 0.632 | 53278 | 14720 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.085 | 1880431 | 15104 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `selector:model` | 0.116 | 983132 | 15104 | `throughput_cache` | 3 | 2912.11 | 0.000 | 0.318 |
| `fixed:balanced` | 0.130 | 1490206 | 15104 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.130 | 1928968 | 15104 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.131 | 1743380 | 15104 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.131 | 1553801 | 15104 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.278 | 51423 | 15104 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.299 | 54059 | 15104 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |
| `selector:rule` | 0.425 | 49470 | 15104 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.086 | 1699411 | 15616 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `selector:model` | 0.116 | 951327 | 15616 | `throughput_cache` | 3 | 2912.11 | 0.000 | 0.318 |
| `fixed:balanced` | 0.130 | 1599121 | 15616 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.130 | 1932423 | 15616 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.131 | 1549492 | 15616 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.132 | 1344712 | 15616 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.285 | 46102 | 15616 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.306 | 48300 | 15616 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |
| `selector:rule` | 0.402 | 53008 | 15616 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.086 | 1529196 | 16128 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `selector:model` | 0.117 | 801565 | 16128 | `throughput_cache` | 3 | 2912.11 | 0.000 | 0.318 |
| `fixed:balanced` | 0.129 | 1587283 | 16128 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.130 | 1611133 | 16128 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.130 | 1541292 | 16128 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.132 | 1150940 | 16128 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.285 | 43074 | 16128 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.312 | 43491 | 16128 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |
| `selector:rule` | 0.416 | 45385 | 16128 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `throughput_churn:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.130 | 7272776 | 13952 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.130 | 6497524 | 13952 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.130 | 6456404 | 13952 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.130 | 5616122 | 13952 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.130 | 5555941 | 13952 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.131 | 5348339 | 13952 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.131 | 5241584 | 13952 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.300 | 60290 | 13952 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.364 | 55339 | 13952 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.130 | 7803655 | 14336 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.130 | 7804095 | 14336 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.130 | 7612462 | 14336 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.130 | 7368378 | 14336 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.130 | 6669381 | 14336 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.131 | 5049368 | 14336 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.131 | 4576145 | 14336 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.309 | 60478 | 14336 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.364 | 59009 | 14336 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.130 | 8135444 | 14848 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.130 | 6796520 | 14848 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.338 | 7842169 | 14976 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.338 | 7733960 | 14976 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.338 | 7023363 | 14976 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.338 | 5803585 | 14976 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.338 | 5680973 | 14976 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.510 | 63086 | 14976 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.572 | 59011 | 14976 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.130 | 7197930 | 15360 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.130 | 7030925 | 15360 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.130 | 6928593 | 15360 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.130 | 5899999 | 15360 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.131 | 5440639 | 15360 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.338 | 5419309 | 15488 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.338 | 4321824 | 15488 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.364 | 56106 | 15360 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |
| `fixed:large_object` | 0.517 | 57331 | 15488 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |

## `throughput_churn:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.130 | 7851062 | 15872 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.130 | 7122801 | 15872 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.130 | 7065612 | 15872 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.130 | 7033360 | 15872 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.130 | 6441028 | 15872 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.131 | 4718492 | 15872 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.313 | 53526 | 15872 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `selector:rule` | 0.338 | 4431985 | 16000 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.351 | 58334 | 15872 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

