# Adaptive Selector Model Evaluation Results

Generated at: `2026-05-11 21:16:19 +0800`

These measurements compare fixed adaptive modes against the measured-cost `model` selector on the generated workload suite. When requested, the legacy `rule` selector is included only as a baseline. Lower score is better.

## Reproduction Command

```bash
python3 tools/evaluate_selector_model.py --bench-runner ./build/bench_runner --iters 20000 --slots 512 --threads 2 --seeds 20001 20002 20003 20004 20005 --mode-window 64 --mode-cooldown 0 --json-out models/selector_evaluation_results.json --markdown-out docs/adaptive_selector_model_results.md --include-rule-baseline
```

## Score Definition

Score is normalized per workload across all compared cases. The metric weights are loaded from the trained objective model summary, so the report uses the same learned objective family as training:

| Metric | Weight | Direction |
|---|---:|---|
| `fragmentation_estimate` | 0.00 | lower is better |
| `mapped_live_ratio` | 0.26 | lower is better |
| `mode_switches` | 0.00 | lower is better |
| `ms` | 0.13 | lower is better |
| `peak_rss_kb` | 0.15 | lower is better |
| `slow_path_ratio` | 0.46 | lower is better |
| `validation_errors` | 0.00 | lower is better |

## Overall Ranking

| Rank | Case | Mean score | Mean ops/sec | Mean peak RSS KB | Mean switches | Best workload count |
|---:|---|---:|---:|---:|---:|---:|
| 1 | `fixed:large_object` | 0.253 | 2445454 | 29131 | 0.0 | 4 |
| 2 | `selector:model` | 0.273 | 1905472 | 27753 | 11.3 | 15 |
| 3 | `fixed:fragmentation_stable` | 0.324 | 2304371 | 29221 | 0.0 | 2 |
| 4 | `fixed:balanced` | 0.346 | 2198259 | 29530 | 0.0 | 0 |
| 5 | `fixed:deterministic_latency` | 0.362 | 2490116 | 29795 | 0.0 | 1 |
| 6 | `fixed:cross_thread` | 0.398 | 2431532 | 30014 | 0.0 | 4 |
| 7 | `fixed:throughput_cache` | 0.406 | 2389145 | 30113 | 0.0 | 2 |
| 8 | `selector:rule` | 0.406 | 2019618 | 29363 | 2.5 | 2 |
| 9 | `fixed:compact_rss` | 0.512 | 1073329 | 27462 | 0.0 | 5 |

## Per-Workload Winners

| Workload | Best overall | Best fixed mode | Model selector |
|---|---|---|---|
| `adaptive_mix:seed20001` | `selector:model` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20002` | `selector:model` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20003` | `selector:model` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20004` | `selector:model` | `fixed:large_object` | `selector:model` |
| `adaptive_mix:seed20005` | `selector:model` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20001` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20002` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20003` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20004` | `fixed:large_object` | `fixed:large_object` | `selector:model` |
| `fragmentation_drift:seed20005` | `selector:rule` | `fixed:large_object` | `selector:model` |
| `large_burst:seed20001` | `fixed:cross_thread` | `fixed:cross_thread` | `selector:model` |
| `large_burst:seed20002` | `fixed:throughput_cache` | `fixed:throughput_cache` | `selector:model` |
| `large_burst:seed20003` | `selector:rule` | `fixed:cross_thread` | `selector:model` |
| `large_burst:seed20004` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `large_burst:seed20005` | `fixed:cross_thread` | `fixed:cross_thread` | `selector:model` |
| `latency_loop:seed20001` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `latency_loop:seed20002` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `latency_loop:seed20003` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `latency_loop:seed20004` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `latency_loop:seed20005` | `fixed:compact_rss` | `fixed:compact_rss` | `selector:model` |
| `remote_queue:seed20001` | `selector:model` | `fixed:large_object` | `selector:model` |
| `remote_queue:seed20002` | `selector:model` | `fixed:balanced` | `selector:model` |
| `remote_queue:seed20003` | `selector:model` | `fixed:large_object` | `selector:model` |
| `remote_queue:seed20004` | `selector:model` | `fixed:fragmentation_stable` | `selector:model` |
| `remote_queue:seed20005` | `selector:model` | `fixed:large_object` | `selector:model` |
| `rss_peak_release:seed20001` | `selector:model` | `fixed:large_object` | `selector:model` |
| `rss_peak_release:seed20002` | `selector:model` | `fixed:large_object` | `selector:model` |
| `rss_peak_release:seed20003` | `selector:model` | `fixed:balanced` | `selector:model` |
| `rss_peak_release:seed20004` | `selector:model` | `fixed:large_object` | `selector:model` |
| `rss_peak_release:seed20005` | `selector:model` | `fixed:balanced` | `selector:model` |
| `throughput_churn:seed20001` | `fixed:cross_thread` | `fixed:cross_thread` | `selector:model` |
| `throughput_churn:seed20002` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `selector:model` |
| `throughput_churn:seed20003` | `fixed:cross_thread` | `fixed:cross_thread` | `selector:model` |
| `throughput_churn:seed20004` | `fixed:throughput_cache` | `fixed:throughput_cache` | `selector:model` |
| `throughput_churn:seed20005` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `selector:model` |

## `adaptive_mix:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.083 | 184645 | 37468 | `compact_rss` | 5 | 121.54 | 0.005 | 0.472 |
| `fixed:large_object` | 0.253 | 213454 | 42804 | `large_object` | 0 | 3949.39 | 0.005 | 0.405 |
| `fixed:fragmentation_stable` | 0.288 | 203453 | 42712 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.451 |
| `selector:rule` | 0.330 | 196015 | 43512 | `compact_rss` | 10 | 5556.80 | 0.005 | 0.408 |
| `fixed:balanced` | 0.380 | 182277 | 43984 | `balanced` | 0 | 5639.60 | 0.005 | 0.448 |
| `fixed:deterministic_latency` | 0.420 | 192228 | 44288 | `deterministic_latency` | 0 | 6757.79 | 0.005 | 0.448 |
| `fixed:cross_thread` | 0.435 | 199241 | 44160 | `cross_thread` | 0 | 7292.58 | 0.005 | 0.451 |
| `fixed:throughput_cache` | 0.457 | 192203 | 44160 | `throughput_cache` | 0 | 7754.44 | 0.005 | 0.456 |
| `fixed:compact_rss` | 0.588 | 73963 | 36508 | `compact_rss` | 0 | 109.39 | 0.005 | 0.983 |

## `adaptive_mix:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.090 | 185547 | 39744 | `compact_rss` | 5 | 121.54 | 0.004 | 0.492 |
| `fixed:large_object` | 0.236 | 199120 | 44236 | `large_object` | 0 | 3945.59 | 0.004 | 0.412 |
| `fixed:fragmentation_stable` | 0.292 | 180554 | 44720 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.467 |
| `selector:rule` | 0.324 | 190040 | 44528 | `compact_rss` | 12 | 5549.20 | 0.004 | 0.446 |
| `fixed:balanced` | 0.360 | 184936 | 45372 | `balanced` | 0 | 5688.21 | 0.004 | 0.462 |
| `fixed:deterministic_latency` | 0.420 | 185936 | 46208 | `deterministic_latency` | 0 | 7049.50 | 0.004 | 0.462 |
| `fixed:cross_thread` | 0.444 | 178084 | 46208 | `cross_thread` | 0 | 7535.67 | 0.004 | 0.467 |
| `fixed:throughput_cache` | 0.460 | 193929 | 46336 | `throughput_cache` | 0 | 8058.30 | 0.004 | 0.471 |
| `fixed:compact_rss` | 0.588 | 73271 | 38628 | `compact_rss` | 0 | 109.39 | 0.004 | 1.004 |

## `adaptive_mix:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.086 | 182478 | 39372 | `compact_rss` | 5 | 121.54 | 0.005 | 0.482 |
| `fixed:large_object` | 0.254 | 210819 | 44368 | `large_object` | 0 | 3946.35 | 0.005 | 0.417 |
| `fixed:fragmentation_stable` | 0.290 | 191173 | 44316 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.460 |
| `selector:rule` | 0.350 | 187665 | 45736 | `compact_rss` | 12 | 5550.72 | 0.005 | 0.425 |
| `fixed:balanced` | 0.372 | 196177 | 45696 | `balanced` | 0 | 5590.98 | 0.005 | 0.456 |
| `fixed:deterministic_latency` | 0.415 | 200876 | 45696 | `deterministic_latency` | 0 | 6903.64 | 0.005 | 0.456 |
| `fixed:cross_thread` | 0.428 | 199522 | 45568 | `cross_thread` | 0 | 7292.58 | 0.005 | 0.459 |
| `fixed:throughput_cache` | 0.448 | 213501 | 45696 | `throughput_cache` | 0 | 7839.52 | 0.005 | 0.463 |
| `fixed:compact_rss` | 0.588 | 73256 | 38240 | `compact_rss` | 0 | 109.39 | 0.005 | 0.994 |

## `adaptive_mix:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.080 | 195272 | 37868 | `compact_rss` | 5 | 121.54 | 0.004 | 0.475 |
| `fixed:large_object` | 0.250 | 222072 | 42920 | `large_object` | 0 | 3944.83 | 0.004 | 0.408 |
| `fixed:fragmentation_stable` | 0.289 | 220843 | 43212 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.453 |
| `selector:rule` | 0.328 | 210020 | 43612 | `compact_rss` | 10 | 5549.20 | 0.004 | 0.413 |
| `fixed:balanced` | 0.365 | 204892 | 43892 | `balanced` | 0 | 5590.98 | 0.004 | 0.449 |
| `fixed:deterministic_latency` | 0.411 | 223770 | 44416 | `deterministic_latency` | 0 | 6806.41 | 0.004 | 0.449 |
| `fixed:cross_thread` | 0.441 | 205809 | 44544 | `cross_thread` | 0 | 7316.89 | 0.004 | 0.453 |
| `fixed:throughput_cache` | 0.449 | 220271 | 44416 | `throughput_cache` | 0 | 7717.98 | 0.004 | 0.457 |
| `fixed:compact_rss` | 0.588 | 83169 | 37124 | `compact_rss` | 0 | 109.39 | 0.004 | 0.986 |

## `adaptive_mix:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.083 | 177105 | 38344 | `compact_rss` | 5 | 121.54 | 0.004 | 0.486 |
| `fixed:large_object` | 0.249 | 203905 | 43392 | `large_object` | 0 | 3944.83 | 0.004 | 0.419 |
| `fixed:fragmentation_stable` | 0.275 | 204668 | 43232 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.461 |
| `selector:rule` | 0.306 | 206843 | 43296 | `compact_rss` | 8 | 5549.96 | 0.004 | 0.425 |
| `fixed:balanced` | 0.353 | 206599 | 44280 | `balanced` | 0 | 5590.98 | 0.004 | 0.458 |
| `fixed:deterministic_latency` | 0.409 | 201876 | 44928 | `deterministic_latency` | 0 | 6806.41 | 0.004 | 0.458 |
| `fixed:cross_thread` | 0.433 | 207925 | 45184 | `cross_thread` | 0 | 7353.35 | 0.004 | 0.461 |
| `fixed:throughput_cache` | 0.447 | 210741 | 45056 | `throughput_cache` | 0 | 7742.29 | 0.004 | 0.466 |
| `fixed:compact_rss` | 0.588 | 78737 | 37600 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |

## `fragmentation_drift:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.299 | 129005 | 52676 | `large_object` | 0 | 3640.98 | 0.002 | 8.153 |
| `selector:rule` | 0.466 | 124256 | 53900 | `large_object` | 3 | 3968.43 | 0.002 | 8.368 |
| `selector:model` | 0.565 | 117132 | 47824 | `compact_rss` | 1 | 394.63 | 0.002 | 9.185 |
| `fixed:compact_rss` | 0.588 | 111658 | 47156 | `compact_rss` | 0 | 108.86 | 0.002 | 9.200 |
| `fixed:fragmentation_stable` | 0.832 | 115258 | 53028 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.143 |
| `fixed:cross_thread` | 0.919 | 118196 | 55424 | `cross_thread` | 0 | 4721.97 | 0.002 | 9.143 |
| `fixed:balanced` | 0.927 | 112952 | 54364 | `balanced` | 0 | 4449.81 | 0.002 | 9.143 |
| `fixed:throughput_cache` | 0.930 | 117181 | 55552 | `throughput_cache` | 0 | 4721.97 | 0.002 | 9.143 |
| `fixed:deterministic_latency` | 0.935 | 116188 | 55424 | `deterministic_latency` | 0 | 4721.97 | 0.002 | 9.143 |

## `fragmentation_drift:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.274 | 123909 | 53300 | `large_object` | 0 | 3735.39 | 0.002 | 8.562 |
| `selector:rule` | 0.472 | 120927 | 53884 | `large_object` | 3 | 4007.55 | 0.002 | 8.942 |
| `selector:model` | 0.505 | 120927 | 48676 | `compact_rss` | 1 | 367.42 | 0.002 | 9.713 |
| `fixed:compact_rss` | 0.520 | 116732 | 47900 | `compact_rss` | 0 | 108.86 | 0.002 | 9.725 |
| `fixed:fragmentation_stable` | 0.727 | 120788 | 54316 | `fragmentation_stable` | 0 | 3619.72 | 0.002 | 9.615 |
| `fixed:balanced` | 0.873 | 112873 | 54976 | `balanced` | 0 | 4871.65 | 0.002 | 9.615 |
| `fixed:cross_thread` | 0.888 | 116848 | 56576 | `cross_thread` | 0 | 5361.54 | 0.002 | 9.615 |
| `fixed:throughput_cache` | 0.892 | 116421 | 56576 | `throughput_cache` | 0 | 5361.54 | 0.002 | 9.615 |
| `fixed:deterministic_latency` | 0.954 | 109741 | 56448 | `deterministic_latency` | 0 | 5361.54 | 0.002 | 9.615 |

## `fragmentation_drift:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.286 | 126616 | 53968 | `large_object` | 0 | 3668.20 | 0.002 | 8.611 |
| `selector:rule` | 0.473 | 121823 | 54656 | `large_object` | 5 | 3940.36 | 0.002 | 8.889 |
| `fixed:compact_rss` | 0.555 | 114868 | 47972 | `compact_rss` | 0 | 108.86 | 0.002 | 9.636 |
| `selector:model` | 0.556 | 117115 | 48708 | `compact_rss` | 1 | 394.63 | 0.002 | 9.621 |
| `fixed:fragmentation_stable` | 0.842 | 111334 | 54264 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.568 |
| `fixed:balanced` | 0.889 | 115805 | 55828 | `balanced` | 0 | 4749.18 | 0.002 | 9.568 |
| `fixed:throughput_cache` | 0.896 | 119508 | 56960 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.568 |
| `fixed:cross_thread` | 0.904 | 118273 | 56832 | `cross_thread` | 0 | 5130.21 | 0.002 | 9.568 |
| `fixed:deterministic_latency` | 0.927 | 115989 | 56960 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.568 |

## `fragmentation_drift:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.364 | 116462 | 53104 | `large_object` | 0 | 3680.96 | 0.002 | 8.689 |
| `selector:rule` | 0.427 | 121186 | 54120 | `large_object` | 3 | 3898.68 | 0.002 | 8.830 |
| `selector:model` | 0.570 | 115290 | 48352 | `compact_rss` | 1 | 462.67 | 0.002 | 9.587 |
| `fixed:compact_rss` | 0.588 | 110477 | 47724 | `compact_rss` | 0 | 108.86 | 0.002 | 9.598 |
| `fixed:fragmentation_stable` | 0.832 | 112412 | 53712 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.538 |
| `fixed:throughput_cache` | 0.838 | 126386 | 56064 | `throughput_cache` | 0 | 4980.52 | 0.002 | 9.538 |
| `fixed:deterministic_latency` | 0.841 | 126337 | 56192 | `deterministic_latency` | 0 | 4980.52 | 0.002 | 9.538 |
| `fixed:balanced` | 0.909 | 113511 | 55296 | `balanced` | 0 | 4653.93 | 0.002 | 9.538 |
| `fixed:cross_thread` | 0.915 | 116432 | 56064 | `cross_thread` | 0 | 4980.52 | 0.002 | 9.538 |

## `fragmentation_drift:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:rule` | 0.354 | 113822 | 53496 | `large_object` | 3 | 3845.95 | 0.002 | 9.047 |
| `fixed:large_object` | 0.360 | 112514 | 53256 | `large_object` | 0 | 3680.96 | 0.002 | 9.064 |
| `fixed:compact_rss` | 0.500 | 114765 | 47532 | `compact_rss` | 0 | 108.86 | 0.002 | 10.042 |
| `selector:model` | 0.556 | 108876 | 48056 | `compact_rss` | 1 | 176.90 | 0.002 | 10.023 |
| `fixed:balanced` | 0.813 | 116323 | 55192 | `balanced` | 0 | 4599.49 | 0.002 | 9.936 |
| `fixed:throughput_cache` | 0.819 | 119919 | 56192 | `throughput_cache` | 0 | 4926.09 | 0.002 | 9.936 |
| `fixed:deterministic_latency` | 0.827 | 119221 | 56320 | `deterministic_latency` | 0 | 4926.09 | 0.002 | 9.936 |
| `fixed:fragmentation_stable` | 0.830 | 105101 | 53540 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.936 |
| `fixed:cross_thread` | 0.924 | 107665 | 56192 | `cross_thread` | 0 | 4926.09 | 0.002 | 9.936 |

## `large_burst:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.285 | 339790 | 14080 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.285 | 335704 | 14080 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.286 | 332029 | 14080 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.286 | 331345 | 14080 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.286 | 319562 | 14080 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.296 | 609908 | 14080 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `selector:rule` | 0.399 | 3136678 | 14208 | `large_object` | 1 | 1143.87 | 0.000 | 0.021 |
| `selector:model` | 0.538 | 593592 | 14208 | `throughput_cache` | 64 | 1198.66 | 0.000 | 0.480 |
| `fixed:compact_rss` | 0.588 | 48855 | 14080 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.288 | 333505 | 14592 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.288 | 324700 | 14592 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.292 | 281093 | 14592 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `selector:rule` | 0.399 | 3903772 | 14720 | `large_object` | 1 | 1143.87 | 0.000 | 0.021 |
| `fixed:fragmentation_stable` | 0.439 | 332795 | 14720 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.440 | 320549 | 14720 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.448 | 651866 | 14720 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `selector:model` | 0.538 | 663375 | 14720 | `throughput_cache` | 64 | 1198.66 | 0.000 | 0.480 |
| `fixed:compact_rss` | 0.588 | 54395 | 14592 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:rule` | 0.248 | 3867786 | 15232 | `large_object` | 1 | 1143.87 | 0.000 | 0.021 |
| `fixed:cross_thread` | 0.288 | 321892 | 15232 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.288 | 319658 | 15232 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.288 | 312984 | 15232 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.289 | 306326 | 15232 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.290 | 283208 | 15232 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.297 | 603240 | 15232 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `selector:model` | 0.387 | 675897 | 15232 | `throughput_cache` | 64 | 1198.66 | 0.000 | 0.480 |
| `fixed:compact_rss` | 0.588 | 51811 | 15232 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.287 | 320238 | 15616 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.287 | 311891 | 15616 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.288 | 304414 | 15616 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.288 | 298313 | 15616 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.290 | 271165 | 15616 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.295 | 676079 | 15616 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `selector:rule` | 0.399 | 3571872 | 15744 | `large_object` | 1 | 1143.87 | 0.000 | 0.021 |
| `selector:model` | 0.537 | 669020 | 15744 | `throughput_cache` | 64 | 1198.66 | 0.000 | 0.480 |
| `fixed:compact_rss` | 0.588 | 49087 | 15616 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.288 | 341788 | 16128 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.288 | 332448 | 16128 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.288 | 330825 | 16128 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.289 | 319434 | 16128 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.289 | 318950 | 16128 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.298 | 582696 | 16128 | `large_object` | 0 | 856.19 | 0.000 | 0.445 |
| `selector:rule` | 0.399 | 3638688 | 16256 | `large_object` | 1 | 1143.87 | 0.000 | 0.021 |
| `selector:model` | 0.540 | 597848 | 16256 | `throughput_cache` | 64 | 1198.66 | 0.000 | 0.480 |
| `fixed:compact_rss` | 0.588 | 55840 | 16128 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `latency_loop:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.004 | 7559079 | 14336 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.261 | 7788447 | 14336 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.275 | 7118607 | 14336 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.275 | 7095837 | 14336 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.276 | 7047410 | 14336 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.279 | 6924727 | 14336 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.283 | 6756696 | 14336 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.358 | 4687446 | 14336 | `large_object` | 1 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.390 | 4136561 | 14336 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.156 | 7524159 | 14976 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.271 | 7266429 | 14848 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.280 | 6890423 | 14848 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.412 | 7750134 | 14976 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.412 | 7731049 | 14976 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.416 | 7572208 | 14976 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.426 | 7074982 | 14976 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.458 | 5939830 | 14976 | `large_object` | 1 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.541 | 4189600 | 14976 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.020 | 7377860 | 15488 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.261 | 8020209 | 15488 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.265 | 7889559 | 15488 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.270 | 7730501 | 15488 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.285 | 7260867 | 15488 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.287 | 7215245 | 15488 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.343 | 5932723 | 15488 | `large_object` | 1 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.374 | 5407098 | 15488 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.390 | 5165411 | 15488 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.017 | 7462652 | 15872 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.261 | 7959643 | 15872 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.269 | 7708923 | 15872 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.270 | 7700080 | 15872 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.273 | 7594215 | 15872 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.296 | 7003108 | 15872 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.299 | 6934882 | 15872 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.345 | 6004316 | 15872 | `large_object` | 1 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.390 | 5307913 | 15872 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.119 | 5169427 | 16512 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.261 | 7752688 | 16512 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.284 | 7063394 | 16512 | `large_object` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.287 | 6988607 | 16512 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.320 | 6206553 | 16512 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.330 | 6011060 | 16512 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:model` | 0.365 | 5387641 | 16512 | `large_object` | 1 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.378 | 5198315 | 16512 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `selector:rule` | 0.390 | 5023232 | 16512 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `remote_queue:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.034 | 511652 | 46464 | `compact_rss` | 3 | 194.47 | 0.185 | 0.022 |
| `fixed:large_object` | 0.047 | 569697 | 46208 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.056 | 544516 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.068 | 516448 | 46336 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `selector:rule` | 0.072 | 552750 | 46464 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.112 | 592707 | 46464 | `deterministic_latency` | 0 | 2224.24 | 0.185 | 0.023 |
| `fixed:cross_thread` | 0.207 | 592229 | 47872 | `cross_thread` | 0 | 2661.79 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.419 | 631188 | 48896 | `throughput_cache` | 0 | 6466.09 | 0.185 | 0.026 |
| `fixed:compact_rss` | 0.588 | 149885 | 46080 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |

## `remote_queue:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.042 | 511331 | 46208 | `compact_rss` | 3 | 194.47 | 0.185 | 0.022 |
| `fixed:balanced` | 0.050 | 614331 | 45824 | `balanced` | 0 | 1239.74 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.050 | 585179 | 45952 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:large_object` | 0.051 | 571894 | 45952 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `selector:rule` | 0.070 | 584368 | 46080 | `balanced` | 0 | 1239.74 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.100 | 617633 | 45952 | `deterministic_latency` | 0 | 2236.39 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.230 | 654501 | 47488 | `cross_thread` | 0 | 2771.18 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.417 | 672644 | 48000 | `throughput_cache` | 0 | 6466.09 | 0.185 | 0.025 |
| `fixed:compact_rss` | 0.588 | 154028 | 45824 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |

## `remote_queue:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.043 | 529190 | 46592 | `compact_rss` | 3 | 194.47 | 0.185 | 0.022 |
| `fixed:large_object` | 0.051 | 599616 | 46336 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.055 | 556869 | 46208 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.056 | 536510 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `selector:rule` | 0.075 | 529063 | 46464 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.123 | 586594 | 46592 | `deterministic_latency` | 0 | 2224.24 | 0.185 | 0.023 |
| `fixed:cross_thread` | 0.227 | 592248 | 47744 | `cross_thread` | 0 | 2795.49 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.419 | 728941 | 48384 | `throughput_cache` | 0 | 6526.86 | 0.185 | 0.026 |
| `fixed:compact_rss` | 0.597 | 149333 | 46336 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |

## `remote_queue:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.027 | 502976 | 46080 | `compact_rss` | 3 | 194.47 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.049 | 523939 | 45952 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:large_object` | 0.051 | 596145 | 46080 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.052 | 602700 | 45952 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `selector:rule` | 0.069 | 594404 | 46208 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.110 | 629952 | 46208 | `deterministic_latency` | 0 | 2272.86 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.223 | 623263 | 47616 | `cross_thread` | 0 | 2807.64 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.419 | 739324 | 48384 | `throughput_cache` | 0 | 6526.86 | 0.185 | 0.026 |
| `fixed:compact_rss` | 0.588 | 149286 | 45952 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |

## `remote_queue:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.032 | 495853 | 46592 | `compact_rss` | 3 | 194.47 | 0.185 | 0.022 |
| `fixed:large_object` | 0.041 | 609770 | 46336 | `large_object` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.052 | 578859 | 46336 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.057 | 566974 | 46592 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `selector:rule` | 0.068 | 550580 | 46592 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.102 | 667555 | 46592 | `deterministic_latency` | 0 | 2224.24 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.228 | 622356 | 48512 | `cross_thread` | 0 | 2746.87 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.419 | 699747 | 49280 | `throughput_cache` | 0 | 6478.24 | 0.185 | 0.026 |
| `fixed:compact_rss` | 0.588 | 156223 | 46336 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |

## `rss_peak_release:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.009 | 1099457 | 14208 | `fragmentation_stable` | 3 | 190.51 | 0.000 | 0.055 |
| `fixed:large_object` | 0.170 | 1690768 | 14208 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.170 | 1605278 | 14208 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.258 | 1550175 | 14208 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.261 | 1654948 | 14208 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.261 | 1589368 | 14208 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.261 | 1547278 | 14208 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.588 | 48884 | 14208 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `selector:rule` | 0.839 | 49104 | 14208 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.009 | 1172297 | 14720 | `fragmentation_stable` | 3 | 190.51 | 0.000 | 0.055 |
| `fixed:large_object` | 0.171 | 1783141 | 14720 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.172 | 1361731 | 14720 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.258 | 1951195 | 14720 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.261 | 2055147 | 14720 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.261 | 2018405 | 14720 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.261 | 1914799 | 14720 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.584 | 50690 | 14720 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `selector:rule` | 0.840 | 49086 | 14720 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.160 | 1103455 | 15360 | `fragmentation_stable` | 3 | 190.51 | 0.000 | 0.055 |
| `fixed:balanced` | 0.259 | 1716870 | 15232 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.261 | 1949765 | 15232 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.262 | 1360865 | 15232 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.322 | 1762643 | 15360 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.322 | 1617968 | 15360 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.412 | 1797302 | 15360 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.739 | 46411 | 15360 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `selector:rule` | 0.982 | 49668 | 15360 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.010 | 1038610 | 15744 | `fragmentation_stable` | 3 | 190.51 | 0.000 | 0.055 |
| `fixed:large_object` | 0.170 | 1786863 | 15744 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.171 | 1647647 | 15744 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.258 | 1801248 | 15744 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.261 | 1844654 | 15744 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.261 | 1722579 | 15744 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.261 | 1562548 | 15744 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.588 | 47569 | 15744 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `selector:rule` | 0.834 | 49884 | 15744 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `rss_peak_release:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `selector:model` | 0.160 | 1198063 | 16384 | `fragmentation_stable` | 3 | 190.51 | 0.000 | 0.055 |
| `fixed:balanced` | 0.259 | 1640153 | 16256 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.261 | 1923779 | 16256 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.262 | 1614492 | 16256 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:fragmentation_stable` | 0.322 | 1803162 | 16384 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:large_object` | 0.322 | 1439803 | 16384 | `large_object` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.412 | 1960365 | 16384 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.588 | 49120 | 16256 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `selector:rule` | 0.984 | 51768 | 16384 | `compact_rss` | 3 | 5307.11 | 0.000 | 1.143 |

## `throughput_churn:seed20001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.261 | 6811525 | 13952 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.261 | 6611317 | 13952 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.261 | 5691955 | 13952 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.261 | 5678027 | 13952 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.261 | 5453986 | 13952 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.412 | 6876373 | 14080 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.412 | 5263027 | 14080 | `fragmentation_stable` | 2 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.412 | 4792663 | 14080 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.588 | 51574 | 13952 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.261 | 7291076 | 14464 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.261 | 7167249 | 14464 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.261 | 6825113 | 14464 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.261 | 6345076 | 14464 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.261 | 6178687 | 14464 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.261 | 6091826 | 14464 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.261 | 5554951 | 14464 | `fragmentation_stable` | 2 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.261 | 5153272 | 14464 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.588 | 54140 | 14464 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.261 | 6929375 | 15104 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.261 | 6491187 | 15104 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.261 | 6092762 | 15104 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.261 | 5684483 | 15104 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.261 | 5474595 | 15104 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.261 | 5361786 | 15104 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.261 | 5038608 | 15104 | `fragmentation_stable` | 2 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.261 | 4872250 | 15104 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.588 | 46679 | 15104 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.261 | 7313315 | 15616 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.261 | 7309885 | 15616 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.261 | 7041076 | 15616 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.261 | 6498729 | 15616 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.261 | 6241434 | 15616 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.261 | 5857151 | 15616 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.261 | 4514324 | 15616 | `fragmentation_stable` | 2 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.262 | 4309892 | 15616 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.588 | 54970 | 15616 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed20005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.261 | 8027439 | 16000 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.261 | 7490964 | 16000 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.261 | 6901103 | 16000 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.261 | 6700504 | 16000 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.261 | 6594683 | 16000 | `large_object` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:model` | 0.261 | 5501653 | 16000 | `fragmentation_stable` | 2 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.261 | 4838431 | 16000 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `selector:rule` | 0.412 | 4963754 | 16128 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.588 | 53670 | 16000 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

