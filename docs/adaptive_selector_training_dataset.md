# Adaptive Selector Model Evaluation Results

Generated at: `2026-05-11 22:00:46 +0800`

These measurements compare fixed adaptive modes against the measured-cost `model` selector on the generated workload suite. When requested, the legacy `rule` selector is included only as a baseline. Lower score is better.

## Reproduction Command

```bash
python3 tools/evaluate_selector_model.py --bench-runner ./build/bench_runner --iters 20000 --slots 512 --threads 2 --seeds 10001 10002 10003 10004 10005 10006 10007 10008 10009 10010 10011 10012 10013 10014 10015 10016 10017 10018 10019 10020 --mode-window 64 --mode-cooldown 0 --json-out models/selector_training_dataset.json --markdown-out docs/adaptive_selector_training_dataset.md --fixed-only
```

## Score Definition

Score is normalized per workload across all compared cases. The metric weights are loaded from the trained objective model summary, so the report uses the same learned objective family as training:

| Metric | Weight | Direction |
|---|---:|---|
| `fragmentation_estimate` | 0.26 | lower is better |
| `mapped_live_ratio` | 0.25 | lower is better |
| `mode_switches` | 0.01 | lower is better |
| `ms` | 0.26 | lower is better |
| `peak_rss_kb` | 0.04 | lower is better |
| `slow_path_ratio` | 0.16 | lower is better |
| `validation_errors` | 0.01 | lower is better |

## Overall Ranking

| Rank | Case | Mean score | Mean ops/sec | Mean peak RSS KB | Mean switches | Best workload count |
|---:|---|---:|---:|---:|---:|---:|
| 1 | `fixed:fragmentation_stable` | 0.280 | 2257987 | 30567 | 0.0 | 56 |
| 2 | `fixed:balanced` | 0.306 | 2295467 | 30885 | 0.0 | 17 |
| 3 | `fixed:deterministic_latency` | 0.309 | 2445135 | 31154 | 0.0 | 9 |
| 4 | `fixed:cross_thread` | 0.328 | 2409725 | 31351 | 0.0 | 12 |
| 5 | `fixed:throughput_cache` | 0.338 | 2461850 | 31433 | 0.0 | 8 |
| 6 | `fixed:large_object` | 0.380 | 1041142 | 28651 | 0.0 | 26 |
| 7 | `fixed:compact_rss` | 0.422 | 1108496 | 28844 | 0.0 | 12 |

## Per-Workload Winners

| Workload | Best overall | Best fixed mode | Model selector |
|---|---|---|---|
| `adaptive_mix:seed10001` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10002` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10003` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10004` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10005` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10006` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10007` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10008` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10009` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10010` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10011` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10012` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10013` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10014` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `adaptive_mix:seed10016` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10017` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10018` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10019` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `adaptive_mix:seed10020` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `fragmentation_drift:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10002` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10003` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10004` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10005` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10006` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10007` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10008` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10009` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10010` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10011` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10012` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10014` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10015` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10016` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10017` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `fragmentation_drift:seed10018` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10019` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `fragmentation_drift:seed10020` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `large_burst:seed10001` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10002` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10003` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10004` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10005` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10006` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10007` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10008` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `large_burst:seed10009` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10010` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10011` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10012` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10013` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10014` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `large_burst:seed10015` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `large_burst:seed10016` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `large_burst:seed10017` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `large_burst:seed10018` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `large_burst:seed10019` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `large_burst:seed10020` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `latency_loop:seed10001` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10002` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `latency_loop:seed10003` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `latency_loop:seed10004` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10005` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10006` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10007` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10008` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10009` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10010` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10011` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10012` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `latency_loop:seed10013` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10014` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10015` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10016` | `fixed:compact_rss` | `fixed:compact_rss` | `n/a` |
| `latency_loop:seed10017` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10018` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `latency_loop:seed10019` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `latency_loop:seed10020` | `fixed:large_object` | `fixed:large_object` | `n/a` |
| `remote_queue:seed10001` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10002` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10003` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10004` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10005` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10006` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10007` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10008` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10009` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10010` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10011` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10012` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10013` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10014` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10015` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10016` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10017` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10018` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `remote_queue:seed10019` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `remote_queue:seed10020` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10001` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10002` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10003` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10004` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10005` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10006` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10007` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10008` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10009` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10010` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10011` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10012` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10013` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10014` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10015` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10016` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10017` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10018` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10019` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `rss_peak_release:seed10020` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `throughput_churn:seed10001` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10002` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `throughput_churn:seed10003` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `throughput_churn:seed10004` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `throughput_churn:seed10005` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `throughput_churn:seed10006` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `throughput_churn:seed10007` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `throughput_churn:seed10008` | `fixed:fragmentation_stable` | `fixed:fragmentation_stable` | `n/a` |
| `throughput_churn:seed10009` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `throughput_churn:seed10010` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `throughput_churn:seed10011` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10012` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `throughput_churn:seed10013` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `throughput_churn:seed10014` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10015` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `throughput_churn:seed10016` | `fixed:throughput_cache` | `fixed:throughput_cache` | `n/a` |
| `throughput_churn:seed10017` | `fixed:deterministic_latency` | `fixed:deterministic_latency` | `n/a` |
| `throughput_churn:seed10018` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |
| `throughput_churn:seed10019` | `fixed:balanced` | `fixed:balanced` | `n/a` |
| `throughput_churn:seed10020` | `fixed:cross_thread` | `fixed:cross_thread` | `n/a` |

## `adaptive_mix:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.423 | 190812 | 42300 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.444 |
| `fixed:large_object` | 0.424 | 56898 | 35004 | `large_object` | 0 | 62.29 | 0.004 | 1.254 |
| `fixed:balanced` | 0.478 | 203366 | 43504 | `balanced` | 0 | 5639.60 | 0.005 | 0.440 |
| `fixed:deterministic_latency` | 0.517 | 201664 | 44032 | `deterministic_latency` | 0 | 6757.79 | 0.005 | 0.440 |
| `fixed:cross_thread` | 0.535 | 201331 | 44160 | `cross_thread` | 0 | 7268.27 | 0.005 | 0.444 |
| `fixed:compact_rss` | 0.536 | 78628 | 35580 | `compact_rss` | 0 | 109.39 | 0.005 | 0.978 |
| `fixed:throughput_cache` | 0.562 | 187765 | 44160 | `throughput_cache` | 0 | 7839.52 | 0.005 | 0.449 |

## `adaptive_mix:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.402 | 73682 | 35824 | `large_object` | 0 | 62.29 | 0.004 | 1.276 |
| `fixed:fragmentation_stable` | 0.427 | 183606 | 43144 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.454 |
| `fixed:balanced` | 0.476 | 198334 | 44544 | `balanced` | 0 | 5590.98 | 0.005 | 0.448 |
| `fixed:deterministic_latency` | 0.531 | 184058 | 45440 | `deterministic_latency` | 0 | 6855.03 | 0.005 | 0.448 |
| `fixed:cross_thread` | 0.535 | 199572 | 45312 | `cross_thread` | 0 | 7365.51 | 0.005 | 0.452 |
| `fixed:throughput_cache` | 0.566 | 181285 | 45184 | `throughput_cache` | 0 | 7875.99 | 0.005 | 0.457 |
| `fixed:compact_rss` | 0.638 | 69633 | 37176 | `compact_rss` | 0 | 109.39 | 0.005 | 0.987 |

## `adaptive_mix:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.169 | 215688 | 44164 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.458 |
| `fixed:balanced` | 0.213 | 235469 | 44868 | `balanced` | 0 | 5590.98 | 0.004 | 0.454 |
| `fixed:deterministic_latency` | 0.265 | 224916 | 45440 | `deterministic_latency` | 0 | 6903.64 | 0.004 | 0.454 |
| `fixed:cross_thread` | 0.279 | 230307 | 45440 | `cross_thread` | 0 | 7450.59 | 0.004 | 0.458 |
| `fixed:throughput_cache` | 0.294 | 230132 | 45440 | `throughput_cache` | 0 | 7875.99 | 0.004 | 0.462 |
| `fixed:compact_rss` | 0.364 | 90548 | 38256 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:large_object` | 0.424 | 87857 | 36908 | `large_object` | 0 | 62.29 | 0.004 | 1.274 |

## `adaptive_mix:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.423 | 212705 | 44288 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.455 |
| `fixed:large_object` | 0.424 | 77768 | 36328 | `large_object` | 0 | 62.29 | 0.004 | 1.277 |
| `fixed:balanced` | 0.485 | 212538 | 45184 | `balanced` | 0 | 5688.21 | 0.005 | 0.451 |
| `fixed:deterministic_latency` | 0.517 | 221251 | 45696 | `deterministic_latency` | 0 | 6806.41 | 0.005 | 0.451 |
| `fixed:cross_thread` | 0.538 | 215651 | 45440 | `cross_thread` | 0 | 7341.20 | 0.005 | 0.456 |
| `fixed:throughput_cache` | 0.553 | 219475 | 45440 | `throughput_cache` | 0 | 7875.99 | 0.005 | 0.460 |
| `fixed:compact_rss` | 0.629 | 79518 | 37412 | `compact_rss` | 0 | 109.39 | 0.005 | 0.991 |

## `adaptive_mix:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.174 | 201428 | 43232 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.455 |
| `fixed:balanced` | 0.233 | 205230 | 44288 | `balanced` | 0 | 5639.60 | 0.004 | 0.451 |
| `fixed:cross_thread` | 0.272 | 226495 | 44672 | `cross_thread` | 0 | 7268.27 | 0.004 | 0.455 |
| `fixed:deterministic_latency` | 0.272 | 208103 | 44544 | `deterministic_latency` | 0 | 6903.64 | 0.004 | 0.451 |
| `fixed:throughput_cache` | 0.291 | 224061 | 44544 | `throughput_cache` | 0 | 7790.91 | 0.004 | 0.460 |
| `fixed:compact_rss` | 0.376 | 82041 | 37020 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:large_object` | 0.405 | 85951 | 35900 | `large_object` | 0 | 62.29 | 0.004 | 1.270 |

## `adaptive_mix:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.166 | 181745 | 43252 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.456 |
| `fixed:balanced` | 0.224 | 182859 | 43776 | `balanced` | 0 | 5590.98 | 0.004 | 0.452 |
| `fixed:deterministic_latency` | 0.259 | 192242 | 44544 | `deterministic_latency` | 0 | 6806.41 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.274 | 195014 | 44288 | `cross_thread` | 0 | 7353.35 | 0.004 | 0.456 |
| `fixed:throughput_cache` | 0.294 | 188573 | 44416 | `throughput_cache` | 0 | 7778.75 | 0.004 | 0.460 |
| `fixed:compact_rss` | 0.373 | 70174 | 37436 | `compact_rss` | 0 | 109.39 | 0.004 | 0.989 |
| `fixed:large_object` | 0.424 | 69595 | 36464 | `large_object` | 0 | 62.29 | 0.004 | 1.267 |

## `adaptive_mix:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.192 | 166658 | 42984 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.453 |
| `fixed:balanced` | 0.223 | 209041 | 43904 | `balanced` | 0 | 5590.98 | 0.004 | 0.448 |
| `fixed:deterministic_latency` | 0.258 | 209417 | 44288 | `deterministic_latency` | 0 | 6563.32 | 0.004 | 0.448 |
| `fixed:throughput_cache` | 0.291 | 205922 | 44032 | `throughput_cache` | 0 | 7474.90 | 0.004 | 0.456 |
| `fixed:cross_thread` | 0.301 | 171616 | 44160 | `cross_thread` | 0 | 7049.50 | 0.004 | 0.452 |
| `fixed:compact_rss` | 0.353 | 71188 | 37156 | `compact_rss` | 0 | 109.39 | 0.004 | 0.986 |
| `fixed:large_object` | 0.424 | 66564 | 35768 | `large_object` | 0 | 62.29 | 0.004 | 1.257 |

## `adaptive_mix:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.157 | 185007 | 43772 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.464 |
| `fixed:balanced` | 0.228 | 178543 | 45056 | `balanced` | 0 | 5736.83 | 0.004 | 0.460 |
| `fixed:cross_thread` | 0.275 | 189506 | 45440 | `cross_thread` | 0 | 7426.28 | 0.004 | 0.464 |
| `fixed:deterministic_latency` | 0.277 | 166359 | 45568 | `deterministic_latency` | 0 | 6855.03 | 0.004 | 0.460 |
| `fixed:throughput_cache` | 0.292 | 185017 | 45312 | `throughput_cache` | 0 | 7839.52 | 0.004 | 0.468 |
| `fixed:compact_rss` | 0.375 | 67283 | 37508 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:large_object` | 0.410 | 69722 | 36336 | `large_object` | 0 | 62.29 | 0.004 | 1.282 |

## `adaptive_mix:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.161 | 193782 | 44472 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.465 |
| `fixed:balanced` | 0.222 | 193490 | 45468 | `balanced` | 0 | 5590.98 | 0.004 | 0.461 |
| `fixed:deterministic_latency` | 0.257 | 206825 | 46080 | `deterministic_latency` | 0 | 6903.64 | 0.004 | 0.461 |
| `fixed:cross_thread` | 0.286 | 191163 | 46464 | `cross_thread` | 0 | 7377.66 | 0.004 | 0.464 |
| `fixed:throughput_cache` | 0.292 | 202143 | 46336 | `throughput_cache` | 0 | 7827.37 | 0.004 | 0.468 |
| `fixed:compact_rss` | 0.338 | 78672 | 38832 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:large_object` | 0.424 | 71399 | 37596 | `large_object` | 0 | 62.29 | 0.004 | 1.282 |

## `adaptive_mix:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.166 | 201372 | 43904 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.461 |
| `fixed:balanced` | 0.221 | 204079 | 44336 | `balanced` | 0 | 5590.98 | 0.004 | 0.457 |
| `fixed:deterministic_latency` | 0.258 | 218148 | 45312 | `deterministic_latency` | 0 | 6952.26 | 0.004 | 0.457 |
| `fixed:cross_thread` | 0.274 | 218186 | 45312 | `cross_thread` | 0 | 7414.12 | 0.004 | 0.461 |
| `fixed:throughput_cache` | 0.295 | 211172 | 45440 | `throughput_cache` | 0 | 7875.99 | 0.004 | 0.466 |
| `fixed:compact_rss` | 0.365 | 81213 | 37440 | `compact_rss` | 0 | 109.39 | 0.004 | 0.996 |
| `fixed:large_object` | 0.424 | 79082 | 36532 | `large_object` | 0 | 62.29 | 0.004 | 1.269 |

## `adaptive_mix:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.158 | 235051 | 42564 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.452 |
| `fixed:balanced` | 0.221 | 234052 | 43520 | `balanced` | 0 | 5688.21 | 0.004 | 0.448 |
| `fixed:deterministic_latency` | 0.276 | 219598 | 43776 | `deterministic_latency` | 0 | 7049.50 | 0.004 | 0.448 |
| `fixed:cross_thread` | 0.289 | 221896 | 43648 | `cross_thread` | 0 | 7499.20 | 0.004 | 0.452 |
| `fixed:throughput_cache` | 0.289 | 241046 | 43648 | `throughput_cache` | 0 | 7875.99 | 0.004 | 0.456 |
| `fixed:compact_rss` | 0.358 | 88486 | 36840 | `compact_rss` | 0 | 109.39 | 0.004 | 0.984 |
| `fixed:large_object` | 0.424 | 84271 | 35344 | `large_object` | 0 | 62.29 | 0.004 | 1.267 |

## `adaptive_mix:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.170 | 176293 | 43996 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.463 |
| `fixed:balanced` | 0.216 | 198818 | 45608 | `balanced` | 0 | 5639.60 | 0.004 | 0.458 |
| `fixed:deterministic_latency` | 0.275 | 180879 | 46464 | `deterministic_latency` | 0 | 6903.64 | 0.004 | 0.458 |
| `fixed:cross_thread` | 0.282 | 190171 | 46336 | `cross_thread` | 0 | 7353.35 | 0.004 | 0.462 |
| `fixed:throughput_cache` | 0.296 | 190376 | 46336 | `throughput_cache` | 0 | 7766.60 | 0.004 | 0.467 |
| `fixed:compact_rss` | 0.369 | 71584 | 38180 | `compact_rss` | 0 | 109.39 | 0.004 | 0.997 |
| `fixed:large_object` | 0.424 | 70484 | 37340 | `large_object` | 0 | 62.29 | 0.004 | 1.272 |

## `adaptive_mix:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.173 | 155619 | 43264 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.467 |
| `fixed:balanced` | 0.212 | 180070 | 43852 | `balanced` | 0 | 5590.98 | 0.004 | 0.462 |
| `fixed:cross_thread` | 0.279 | 173494 | 44800 | `cross_thread` | 0 | 7353.35 | 0.004 | 0.466 |
| `fixed:deterministic_latency` | 0.282 | 148543 | 45056 | `deterministic_latency` | 0 | 6806.41 | 0.004 | 0.462 |
| `fixed:throughput_cache` | 0.289 | 179349 | 44800 | `throughput_cache` | 0 | 7778.75 | 0.004 | 0.470 |
| `fixed:large_object` | 0.370 | 65528 | 36264 | `large_object` | 0 | 62.29 | 0.004 | 1.276 |
| `fixed:compact_rss` | 0.375 | 56295 | 37112 | `compact_rss` | 0 | 109.39 | 0.004 | 0.998 |

## `adaptive_mix:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.161 | 188880 | 42112 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.449 |
| `fixed:balanced` | 0.232 | 174489 | 42752 | `balanced` | 0 | 5590.98 | 0.004 | 0.445 |
| `fixed:deterministic_latency` | 0.260 | 182370 | 42880 | `deterministic_latency` | 0 | 6611.94 | 0.004 | 0.445 |
| `fixed:cross_thread` | 0.279 | 183568 | 43136 | `cross_thread` | 0 | 7171.04 | 0.004 | 0.449 |
| `fixed:throughput_cache` | 0.289 | 193538 | 43008 | `throughput_cache` | 0 | 7717.98 | 0.004 | 0.453 |
| `fixed:compact_rss` | 0.357 | 70242 | 36100 | `compact_rss` | 0 | 109.39 | 0.004 | 0.983 |
| `fixed:large_object` | 0.424 | 66670 | 34704 | `large_object` | 0 | 62.29 | 0.004 | 1.260 |

## `adaptive_mix:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.424 | 80601 | 34932 | `large_object` | 0 | 62.29 | 0.004 | 1.264 |
| `fixed:fragmentation_stable` | 0.430 | 215453 | 42356 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.450 |
| `fixed:balanced` | 0.486 | 222964 | 43392 | `balanced` | 0 | 5639.60 | 0.005 | 0.445 |
| `fixed:deterministic_latency` | 0.526 | 219777 | 43776 | `deterministic_latency` | 0 | 6757.79 | 0.005 | 0.445 |
| `fixed:cross_thread` | 0.544 | 223171 | 44032 | `cross_thread` | 0 | 7304.74 | 0.005 | 0.450 |
| `fixed:throughput_cache` | 0.553 | 234024 | 43904 | `throughput_cache` | 0 | 7778.75 | 0.005 | 0.454 |
| `fixed:compact_rss` | 0.623 | 83857 | 36020 | `compact_rss` | 0 | 109.39 | 0.005 | 0.983 |

## `adaptive_mix:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 227851 | 43144 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.457 |
| `fixed:balanced` | 0.224 | 229778 | 43904 | `balanced` | 0 | 5590.98 | 0.004 | 0.452 |
| `fixed:deterministic_latency` | 0.262 | 231970 | 44672 | `deterministic_latency` | 0 | 6709.18 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.276 | 237658 | 44544 | `cross_thread` | 0 | 7231.81 | 0.004 | 0.456 |
| `fixed:throughput_cache` | 0.289 | 241990 | 44544 | `throughput_cache` | 0 | 7705.83 | 0.004 | 0.461 |
| `fixed:compact_rss` | 0.365 | 90877 | 37424 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:large_object` | 0.424 | 88417 | 36380 | `large_object` | 0 | 62.29 | 0.004 | 1.264 |

## `adaptive_mix:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.175 | 206411 | 42752 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.457 |
| `fixed:balanced` | 0.220 | 232187 | 43904 | `balanced` | 0 | 5590.98 | 0.004 | 0.453 |
| `fixed:deterministic_latency` | 0.270 | 216572 | 44416 | `deterministic_latency` | 0 | 6709.18 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.299 | 200201 | 44544 | `cross_thread` | 0 | 7171.04 | 0.004 | 0.456 |
| `fixed:throughput_cache` | 0.302 | 212491 | 44288 | `throughput_cache` | 0 | 7559.98 | 0.004 | 0.461 |
| `fixed:compact_rss` | 0.376 | 80645 | 36532 | `compact_rss` | 0 | 109.39 | 0.004 | 0.990 |
| `fixed:large_object` | 0.416 | 82293 | 35652 | `large_object` | 0 | 62.29 | 0.004 | 1.259 |

## `adaptive_mix:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.168 | 221009 | 43264 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.448 |
| `fixed:balanced` | 0.224 | 226353 | 43904 | `balanced` | 0 | 5639.60 | 0.004 | 0.444 |
| `fixed:deterministic_latency` | 0.271 | 216165 | 44416 | `deterministic_latency` | 0 | 6806.41 | 0.004 | 0.444 |
| `fixed:throughput_cache` | 0.289 | 239221 | 44288 | `throughput_cache` | 0 | 7803.06 | 0.004 | 0.452 |
| `fixed:cross_thread` | 0.296 | 207048 | 44416 | `cross_thread` | 0 | 7341.20 | 0.004 | 0.447 |
| `fixed:compact_rss` | 0.378 | 84024 | 37224 | `compact_rss` | 0 | 109.39 | 0.004 | 0.981 |
| `fixed:large_object` | 0.420 | 84989 | 35568 | `large_object` | 0 | 62.29 | 0.004 | 1.260 |

## `adaptive_mix:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.168 | 190090 | 43008 | `fragmentation_stable` | 0 | 3840.76 | 0.004 | 0.455 |
| `fixed:balanced` | 0.231 | 187445 | 43892 | `balanced` | 0 | 5639.60 | 0.004 | 0.450 |
| `fixed:deterministic_latency` | 0.270 | 188488 | 44544 | `deterministic_latency` | 0 | 6806.41 | 0.004 | 0.450 |
| `fixed:cross_thread` | 0.277 | 206131 | 44544 | `cross_thread` | 0 | 7414.12 | 0.004 | 0.454 |
| `fixed:throughput_cache` | 0.290 | 209285 | 44544 | `throughput_cache` | 0 | 7875.99 | 0.004 | 0.459 |
| `fixed:compact_rss` | 0.352 | 79752 | 37460 | `compact_rss` | 0 | 109.39 | 0.004 | 0.989 |
| `fixed:large_object` | 0.424 | 74809 | 36304 | `large_object` | 0 | 62.29 | 0.004 | 1.258 |

## `adaptive_mix:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.419 | 231995 | 42092 | `fragmentation_stable` | 0 | 3840.76 | 0.005 | 0.451 |
| `fixed:large_object` | 0.424 | 86065 | 35208 | `large_object` | 0 | 62.29 | 0.004 | 1.260 |
| `fixed:balanced` | 0.485 | 223432 | 43064 | `balanced` | 0 | 5590.98 | 0.005 | 0.446 |
| `fixed:deterministic_latency` | 0.522 | 229953 | 43648 | `deterministic_latency` | 0 | 6757.79 | 0.005 | 0.446 |
| `fixed:cross_thread` | 0.543 | 225284 | 43776 | `cross_thread` | 0 | 7268.27 | 0.005 | 0.450 |
| `fixed:throughput_cache` | 0.558 | 224083 | 43648 | `throughput_cache` | 0 | 7693.67 | 0.005 | 0.454 |
| `fixed:compact_rss` | 0.629 | 88179 | 36208 | `compact_rss` | 0 | 109.39 | 0.005 | 0.985 |

## `fragmentation_drift:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.337 | 110909 | 46548 | `large_object` | 0 | 56.13 | 0.000 | 12.102 |
| `fixed:compact_rss` | 0.535 | 107298 | 46820 | `compact_rss` | 0 | 108.86 | 0.002 | 9.483 |
| `fixed:fragmentation_stable` | 0.545 | 115537 | 53284 | `fragmentation_stable` | 0 | 3578.90 | 0.002 | 9.423 |
| `fixed:throughput_cache` | 0.551 | 119134 | 55424 | `throughput_cache` | 0 | 4994.13 | 0.002 | 9.423 |
| `fixed:deterministic_latency` | 0.645 | 114603 | 55296 | `deterministic_latency` | 0 | 4994.13 | 0.002 | 9.423 |
| `fixed:cross_thread` | 0.650 | 114455 | 55552 | `cross_thread` | 0 | 4994.13 | 0.002 | 9.423 |
| `fixed:balanced` | 0.708 | 111118 | 54820 | `balanced` | 0 | 4721.97 | 0.002 | 9.423 |

## `fragmentation_drift:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.354 | 125580 | 46852 | `large_object` | 0 | 56.13 | 0.000 | 12.558 |
| `fixed:fragmentation_stable` | 0.504 | 131928 | 52912 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.625 |
| `fixed:compact_rss` | 0.535 | 122643 | 47216 | `compact_rss` | 0 | 108.86 | 0.002 | 9.685 |
| `fixed:throughput_cache` | 0.550 | 134468 | 56064 | `throughput_cache` | 0 | 5415.97 | 0.002 | 9.625 |
| `fixed:deterministic_latency` | 0.565 | 133782 | 56448 | `deterministic_latency` | 0 | 5415.97 | 0.002 | 9.625 |
| `fixed:balanced` | 0.670 | 127871 | 55012 | `balanced` | 0 | 5034.95 | 0.002 | 9.625 |
| `fixed:cross_thread` | 0.743 | 125662 | 56320 | `cross_thread` | 0 | 5415.97 | 0.002 | 9.625 |

## `fragmentation_drift:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.351 | 121329 | 47636 | `compact_rss` | 0 | 108.86 | 0.002 | 9.668 |
| `fixed:large_object` | 0.424 | 113934 | 47436 | `large_object` | 0 | 56.13 | 0.000 | 12.428 |
| `fixed:fragmentation_stable` | 0.456 | 125002 | 53072 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.630 |
| `fixed:deterministic_latency` | 0.571 | 124096 | 56064 | `deterministic_latency` | 0 | 5211.85 | 0.002 | 9.630 |
| `fixed:throughput_cache` | 0.612 | 122295 | 56064 | `throughput_cache` | 0 | 5211.85 | 0.002 | 9.630 |
| `fixed:balanced` | 0.631 | 120142 | 54584 | `balanced` | 0 | 4721.97 | 0.002 | 9.630 |
| `fixed:cross_thread` | 0.670 | 119788 | 56064 | `cross_thread` | 0 | 5211.85 | 0.002 | 9.630 |

## `fragmentation_drift:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.402 | 121613 | 47360 | `compact_rss` | 0 | 108.86 | 0.002 | 9.430 |
| `fixed:large_object` | 0.424 | 112565 | 47048 | `large_object` | 0 | 56.13 | 0.000 | 12.243 |
| `fixed:throughput_cache` | 0.550 | 131941 | 56064 | `throughput_cache` | 0 | 4994.13 | 0.002 | 9.343 |
| `fixed:deterministic_latency` | 0.587 | 128984 | 56448 | `deterministic_latency` | 0 | 4994.13 | 0.002 | 9.343 |
| `fixed:fragmentation_stable` | 0.588 | 122761 | 54112 | `fragmentation_stable` | 0 | 3633.33 | 0.002 | 9.343 |
| `fixed:balanced` | 0.619 | 125297 | 55632 | `balanced` | 0 | 4776.40 | 0.002 | 9.343 |
| `fixed:cross_thread` | 0.680 | 121678 | 56192 | `cross_thread` | 0 | 4994.13 | 0.002 | 9.343 |

## `fragmentation_drift:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.328 | 120737 | 46908 | `compact_rss` | 0 | 108.86 | 0.002 | 9.645 |
| `fixed:large_object` | 0.424 | 99848 | 46648 | `large_object` | 0 | 56.13 | 0.000 | 12.338 |
| `fixed:throughput_cache` | 0.550 | 127970 | 55168 | `throughput_cache` | 0 | 5075.77 | 0.002 | 9.558 |
| `fixed:balanced` | 0.551 | 125222 | 54400 | `balanced` | 0 | 4749.18 | 0.002 | 9.558 |
| `fixed:deterministic_latency` | 0.597 | 122006 | 55424 | `deterministic_latency` | 0 | 5075.77 | 0.002 | 9.558 |
| `fixed:fragmentation_stable` | 0.649 | 107251 | 52968 | `fragmentation_stable` | 0 | 3606.11 | 0.002 | 9.558 |
| `fixed:cross_thread` | 0.759 | 104808 | 55296 | `cross_thread` | 0 | 5075.77 | 0.002 | 9.558 |

## `fragmentation_drift:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.304 | 111637 | 47504 | `compact_rss` | 0 | 108.86 | 0.002 | 9.698 |
| `fixed:large_object` | 0.422 | 105340 | 47044 | `large_object` | 0 | 56.13 | 0.000 | 12.347 |
| `fixed:fragmentation_stable` | 0.469 | 112568 | 53272 | `fragmentation_stable` | 0 | 3483.64 | 0.002 | 9.634 |
| `fixed:deterministic_latency` | 0.592 | 111398 | 55168 | `deterministic_latency` | 0 | 4898.87 | 0.002 | 9.634 |
| `fixed:throughput_cache` | 0.744 | 107178 | 55168 | `throughput_cache` | 0 | 4898.87 | 0.002 | 9.634 |
| `fixed:balanced` | 0.794 | 105281 | 53992 | `balanced` | 0 | 4572.28 | 0.002 | 9.634 |
| `fixed:cross_thread` | 0.806 | 105551 | 55168 | `cross_thread` | 0 | 4898.87 | 0.002 | 9.634 |

## `fragmentation_drift:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.324 | 108434 | 48160 | `compact_rss` | 0 | 108.86 | 0.002 | 9.802 |
| `fixed:large_object` | 0.424 | 95614 | 47988 | `large_object` | 0 | 56.13 | 0.000 | 12.334 |
| `fixed:fragmentation_stable` | 0.524 | 108370 | 53960 | `fragmentation_stable` | 0 | 3510.86 | 0.002 | 9.723 |
| `fixed:throughput_cache` | 0.551 | 112116 | 56064 | `throughput_cache` | 0 | 4871.65 | 0.002 | 9.723 |
| `fixed:balanced` | 0.562 | 110822 | 55808 | `balanced` | 0 | 4762.79 | 0.002 | 9.723 |
| `fixed:deterministic_latency` | 0.580 | 110088 | 56064 | `deterministic_latency` | 0 | 4871.65 | 0.002 | 9.723 |
| `fixed:cross_thread` | 0.594 | 109094 | 56064 | `cross_thread` | 0 | 4871.65 | 0.002 | 9.723 |

## `fragmentation_drift:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.402 | 113077 | 47188 | `large_object` | 0 | 56.13 | 0.000 | 12.432 |
| `fixed:compact_rss` | 0.424 | 115636 | 47364 | `compact_rss` | 0 | 108.86 | 0.002 | 9.642 |
| `fixed:deterministic_latency` | 0.551 | 120397 | 56064 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.585 |
| `fixed:balanced` | 0.607 | 118182 | 55208 | `balanced` | 0 | 4912.48 | 0.002 | 9.585 |
| `fixed:cross_thread` | 0.700 | 115815 | 56064 | `cross_thread` | 0 | 5130.21 | 0.002 | 9.585 |
| `fixed:throughput_cache` | 0.712 | 115420 | 55808 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.585 |
| `fixed:fragmentation_stable` | 0.727 | 112450 | 53480 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.585 |

## `fragmentation_drift:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.338 | 121106 | 47608 | `compact_rss` | 0 | 108.86 | 0.002 | 9.640 |
| `fixed:large_object` | 0.424 | 108405 | 47376 | `large_object` | 0 | 56.13 | 0.000 | 12.423 |
| `fixed:balanced` | 0.545 | 124595 | 54952 | `balanced` | 0 | 4667.53 | 0.002 | 9.583 |
| `fixed:deterministic_latency` | 0.550 | 126236 | 56064 | `deterministic_latency` | 0 | 5102.99 | 0.002 | 9.583 |
| `fixed:throughput_cache` | 0.554 | 126070 | 56448 | `throughput_cache` | 0 | 5102.99 | 0.002 | 9.583 |
| `fixed:fragmentation_stable` | 0.599 | 116162 | 53256 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.583 |
| `fixed:cross_thread` | 0.686 | 116506 | 56448 | `cross_thread` | 0 | 5102.99 | 0.002 | 9.583 |

## `fragmentation_drift:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.305 | 133513 | 47292 | `compact_rss` | 0 | 108.86 | 0.002 | 9.672 |
| `fixed:large_object` | 0.378 | 128179 | 47004 | `large_object` | 0 | 56.13 | 0.000 | 12.264 |
| `fixed:cross_thread` | 0.551 | 134540 | 56064 | `cross_thread` | 0 | 5130.21 | 0.002 | 9.596 |
| `fixed:fragmentation_stable` | 0.612 | 130236 | 53660 | `fragmentation_stable` | 0 | 3606.11 | 0.002 | 9.596 |
| `fixed:deterministic_latency` | 0.622 | 132416 | 56192 | `deterministic_latency` | 0 | 5130.21 | 0.002 | 9.596 |
| `fixed:throughput_cache` | 0.648 | 131638 | 56064 | `throughput_cache` | 0 | 5130.21 | 0.002 | 9.596 |
| `fixed:balanced` | 0.792 | 126922 | 54964 | `balanced` | 0 | 4749.18 | 0.002 | 9.596 |

## `fragmentation_drift:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.424 | 124484 | 46252 | `large_object` | 0 | 56.13 | 0.000 | 12.179 |
| `fixed:compact_rss` | 0.508 | 126084 | 46816 | `compact_rss` | 0 | 108.86 | 0.002 | 9.421 |
| `fixed:throughput_cache` | 0.550 | 141926 | 54784 | `throughput_cache` | 0 | 5211.85 | 0.002 | 9.375 |
| `fixed:cross_thread` | 0.578 | 139905 | 54912 | `cross_thread` | 0 | 5211.85 | 0.002 | 9.375 |
| `fixed:balanced` | 0.583 | 137686 | 53972 | `balanced` | 0 | 4776.40 | 0.002 | 9.375 |
| `fixed:deterministic_latency` | 0.602 | 138217 | 55168 | `deterministic_latency` | 0 | 5211.85 | 0.002 | 9.375 |
| `fixed:fragmentation_stable` | 0.622 | 130551 | 52392 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.375 |

## `fragmentation_drift:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.264 | 105870 | 47384 | `large_object` | 0 | 56.13 | 0.000 | 12.438 |
| `fixed:compact_rss` | 0.412 | 105070 | 47648 | `compact_rss` | 0 | 108.86 | 0.002 | 9.821 |
| `fixed:fragmentation_stable` | 0.535 | 106442 | 53868 | `fragmentation_stable` | 0 | 3524.47 | 0.002 | 9.764 |
| `fixed:throughput_cache` | 0.551 | 108221 | 56576 | `throughput_cache` | 0 | 5266.29 | 0.002 | 9.764 |
| `fixed:balanced` | 0.688 | 104713 | 55148 | `balanced` | 0 | 4939.69 | 0.002 | 9.764 |
| `fixed:cross_thread` | 0.757 | 103700 | 56448 | `cross_thread` | 0 | 5266.29 | 0.002 | 9.764 |
| `fixed:deterministic_latency` | 0.816 | 102475 | 56576 | `deterministic_latency` | 0 | 5266.29 | 0.002 | 9.764 |

## `fragmentation_drift:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.377 | 94613 | 48008 | `large_object` | 0 | 56.13 | 0.000 | 12.445 |
| `fixed:balanced` | 0.526 | 104133 | 54756 | `balanced` | 0 | 4708.36 | 0.002 | 9.772 |
| `fixed:compact_rss` | 0.534 | 92781 | 48216 | `compact_rss` | 0 | 108.86 | 0.002 | 9.817 |
| `fixed:throughput_cache` | 0.557 | 103832 | 56064 | `throughput_cache` | 0 | 5089.38 | 0.002 | 9.772 |
| `fixed:fragmentation_stable` | 0.568 | 99262 | 53876 | `fragmentation_stable` | 0 | 3510.86 | 0.002 | 9.772 |
| `fixed:deterministic_latency` | 0.654 | 99391 | 56064 | `deterministic_latency` | 0 | 5089.38 | 0.002 | 9.772 |
| `fixed:cross_thread` | 0.782 | 94110 | 56192 | `cross_thread` | 0 | 5089.38 | 0.002 | 9.772 |

## `fragmentation_drift:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.320 | 123646 | 47220 | `compact_rss` | 0 | 108.86 | 0.002 | 9.764 |
| `fixed:large_object` | 0.424 | 106428 | 47032 | `large_object` | 0 | 56.13 | 0.000 | 12.413 |
| `fixed:fragmentation_stable` | 0.499 | 126430 | 53756 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.677 |
| `fixed:throughput_cache` | 0.551 | 128306 | 55424 | `throughput_cache` | 0 | 4749.18 | 0.002 | 9.677 |
| `fixed:deterministic_latency` | 0.592 | 124284 | 55424 | `deterministic_latency` | 0 | 4749.18 | 0.002 | 9.677 |
| `fixed:balanced` | 0.605 | 122454 | 55040 | `balanced` | 0 | 4640.32 | 0.002 | 9.677 |
| `fixed:cross_thread` | 0.738 | 112063 | 55552 | `cross_thread` | 0 | 4749.18 | 0.002 | 9.677 |

## `fragmentation_drift:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.363 | 132218 | 47628 | `compact_rss` | 0 | 108.86 | 0.002 | 9.606 |
| `fixed:large_object` | 0.377 | 126081 | 47164 | `large_object` | 0 | 56.13 | 0.000 | 12.323 |
| `fixed:fragmentation_stable` | 0.501 | 135274 | 53888 | `fragmentation_stable` | 0 | 3592.51 | 0.002 | 9.538 |
| `fixed:deterministic_latency` | 0.551 | 137030 | 56064 | `deterministic_latency` | 0 | 5062.17 | 0.002 | 9.538 |
| `fixed:cross_thread` | 0.624 | 133166 | 56192 | `cross_thread` | 0 | 5062.17 | 0.002 | 9.538 |
| `fixed:throughput_cache` | 0.741 | 127369 | 56192 | `throughput_cache` | 0 | 5062.17 | 0.002 | 9.538 |
| `fixed:balanced` | 0.805 | 123920 | 55552 | `balanced` | 0 | 4898.87 | 0.002 | 9.538 |

## `fragmentation_drift:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.402 | 117483 | 47068 | `large_object` | 0 | 56.13 | 0.000 | 12.306 |
| `fixed:compact_rss` | 0.516 | 117373 | 47284 | `compact_rss` | 0 | 108.86 | 0.002 | 9.679 |
| `fixed:fragmentation_stable` | 0.530 | 125395 | 53232 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.608 |
| `fixed:cross_thread` | 0.551 | 128415 | 56064 | `cross_thread` | 0 | 4966.91 | 0.002 | 9.608 |
| `fixed:throughput_cache` | 0.553 | 128294 | 55936 | `throughput_cache` | 0 | 4966.91 | 0.002 | 9.608 |
| `fixed:balanced` | 0.570 | 126518 | 54508 | `balanced` | 0 | 4694.75 | 0.002 | 9.608 |
| `fixed:deterministic_latency` | 0.816 | 116581 | 56064 | `deterministic_latency` | 0 | 4966.91 | 0.002 | 9.608 |

## `fragmentation_drift:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.402 | 127537 | 46844 | `compact_rss` | 0 | 108.86 | 0.002 | 9.504 |
| `fixed:large_object` | 0.424 | 120191 | 46604 | `large_object` | 0 | 56.13 | 0.000 | 11.981 |
| `fixed:fragmentation_stable` | 0.501 | 133860 | 53148 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.455 |
| `fixed:balanced` | 0.540 | 135811 | 54776 | `balanced` | 0 | 4762.79 | 0.002 | 9.455 |
| `fixed:throughput_cache` | 0.564 | 134955 | 55424 | `throughput_cache` | 0 | 4926.09 | 0.002 | 9.455 |
| `fixed:deterministic_latency` | 0.639 | 130204 | 55552 | `deterministic_latency` | 0 | 4926.09 | 0.002 | 9.455 |
| `fixed:cross_thread` | 0.718 | 125523 | 55552 | `cross_thread` | 0 | 4926.09 | 0.002 | 9.455 |

## `fragmentation_drift:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.294 | 112685 | 47460 | `large_object` | 0 | 56.13 | 0.000 | 12.349 |
| `fixed:compact_rss` | 0.374 | 114985 | 47876 | `compact_rss` | 0 | 108.86 | 0.002 | 9.675 |
| `fixed:fragmentation_stable` | 0.464 | 123406 | 53808 | `fragmentation_stable` | 0 | 3565.29 | 0.002 | 9.623 |
| `fixed:balanced` | 0.532 | 123218 | 54824 | `balanced` | 0 | 4817.22 | 0.002 | 9.623 |
| `fixed:cross_thread` | 0.565 | 122187 | 56064 | `cross_thread` | 0 | 5143.81 | 0.002 | 9.623 |
| `fixed:deterministic_latency` | 0.648 | 115534 | 56064 | `deterministic_latency` | 0 | 5143.81 | 0.002 | 9.623 |
| `fixed:throughput_cache` | 0.816 | 103949 | 56064 | `throughput_cache` | 0 | 5143.81 | 0.002 | 9.623 |

## `fragmentation_drift:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.253 | 117970 | 47912 | `large_object` | 0 | 56.13 | 0.000 | 12.070 |
| `fixed:compact_rss` | 0.272 | 127064 | 48112 | `compact_rss` | 0 | 108.86 | 0.002 | 9.574 |
| `fixed:fragmentation_stable` | 0.488 | 123992 | 53432 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.491 |
| `fixed:cross_thread` | 0.605 | 121718 | 55936 | `cross_thread` | 0 | 5239.07 | 0.002 | 9.491 |
| `fixed:deterministic_latency` | 0.640 | 118457 | 55936 | `deterministic_latency` | 0 | 5239.07 | 0.002 | 9.491 |
| `fixed:balanced` | 0.763 | 106398 | 54772 | `balanced` | 0 | 4858.05 | 0.002 | 9.491 |
| `fixed:throughput_cache` | 0.816 | 104317 | 55936 | `throughput_cache` | 0 | 5239.07 | 0.002 | 9.491 |

## `fragmentation_drift:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.196 | 129762 | 47328 | `large_object` | 0 | 56.13 | 0.000 | 12.113 |
| `fixed:compact_rss` | 0.416 | 120935 | 47580 | `compact_rss` | 0 | 108.86 | 0.002 | 9.477 |
| `fixed:fragmentation_stable` | 0.485 | 131295 | 53724 | `fragmentation_stable` | 0 | 3551.68 | 0.002 | 9.406 |
| `fixed:cross_thread` | 0.551 | 133022 | 55808 | `cross_thread` | 0 | 5075.77 | 0.002 | 9.406 |
| `fixed:balanced` | 0.717 | 118178 | 55124 | `balanced` | 0 | 4803.61 | 0.002 | 9.406 |
| `fixed:throughput_cache` | 0.743 | 117473 | 55808 | `throughput_cache` | 0 | 5075.77 | 0.002 | 9.406 |
| `fixed:deterministic_latency` | 0.816 | 112523 | 55936 | `deterministic_latency` | 0 | 5075.77 | 0.002 | 9.406 |

## `large_burst:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 325032 | 14080 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.251 | 322412 | 14080 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.253 | 306638 | 14080 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.254 | 301733 | 14080 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.259 | 276581 | 14080 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.418 | 53037 | 14080 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 51934 | 14080 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 394214 | 14464 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.253 | 368370 | 14464 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.254 | 367729 | 14464 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.257 | 345904 | 14464 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.262 | 314660 | 14464 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.404 | 64465 | 14464 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 60323 | 14464 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 388056 | 14720 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.253 | 365379 | 14720 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.253 | 361297 | 14720 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.255 | 347725 | 14720 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.256 | 341186 | 14720 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.393 | 61630 | 14720 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 55387 | 14720 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 342668 | 15232 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.255 | 311691 | 15232 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.257 | 298549 | 15232 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.257 | 298072 | 15232 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.263 | 265709 | 15232 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.401 | 54026 | 15232 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 49921 | 15232 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.250 | 404482 | 15616 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.255 | 365187 | 15616 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.256 | 361939 | 15616 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.256 | 358890 | 15616 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.261 | 330523 | 15616 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.423 | 61054 | 15616 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 60746 | 15616 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 325237 | 16000 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.255 | 295133 | 16000 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.256 | 290424 | 16000 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.257 | 283033 | 16000 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.257 | 282175 | 16000 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.395 | 52137 | 16000 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 47293 | 16000 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.250 | 342928 | 16384 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.251 | 336609 | 16384 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.252 | 328557 | 16384 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.254 | 313916 | 16384 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.255 | 310443 | 16384 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.424 | 47346 | 16384 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.466 | 51467 | 16384 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 289499 | 16768 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.253 | 274351 | 16768 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.254 | 266206 | 16768 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.257 | 251244 | 16768 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.261 | 237308 | 16768 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.449 | 46208 | 16896 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 44165 | 16768 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.260 | 319018 | 17152 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.288 | 393929 | 17280 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.290 | 378209 | 17280 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.295 | 339716 | 17280 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.295 | 338877 | 17280 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.455 | 56307 | 17280 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.528 | 55010 | 17280 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.250 | 377163 | 17536 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.250 | 375269 | 17536 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.290 | 365339 | 17664 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.290 | 362574 | 17664 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.294 | 339600 | 17664 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.442 | 62790 | 17664 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.528 | 58617 | 17664 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.252 | 388123 | 17920 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.254 | 372328 | 17920 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.288 | 404517 | 18048 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.290 | 390667 | 18048 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.298 | 328579 | 18048 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.462 | 56280 | 18048 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.511 | 59743 | 18048 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.268 | 239539 | 18176 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.288 | 337095 | 18304 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.294 | 301580 | 18304 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.299 | 273632 | 18304 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.301 | 260659 | 18304 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.462 | 48506 | 18304 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.522 | 49602 | 18304 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.250 | 303262 | 18560 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.252 | 295311 | 18560 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.252 | 294923 | 18560 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.254 | 280117 | 18560 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.255 | 279710 | 18560 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.424 | 50513 | 18560 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.489 | 50729 | 18560 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.250 | 316969 | 18944 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.250 | 314650 | 18944 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.291 | 298021 | 19072 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.293 | 288912 | 19072 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.294 | 278277 | 19072 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.424 | 51247 | 19072 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.528 | 44878 | 19072 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 397553 | 19328 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.250 | 397395 | 19328 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.253 | 375380 | 19328 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.297 | 334871 | 19456 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.303 | 298764 | 19456 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.462 | 57093 | 19456 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.506 | 61580 | 19456 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 368287 | 19840 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.252 | 355667 | 19840 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.253 | 342918 | 19840 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.253 | 341377 | 19840 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.255 | 330527 | 19840 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.424 | 51156 | 19840 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.477 | 53487 | 19840 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.252 | 376432 | 20224 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.288 | 394842 | 20352 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.291 | 375406 | 20352 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.299 | 319601 | 20352 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.304 | 291939 | 20352 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.462 | 56399 | 20352 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.508 | 60501 | 20352 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.250 | 398686 | 20736 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.251 | 387761 | 20736 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.252 | 383403 | 20736 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.253 | 372148 | 20736 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.296 | 338068 | 20864 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.462 | 56113 | 20864 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.477 | 58734 | 20736 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 353022 | 21120 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.251 | 345382 | 21120 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.255 | 319407 | 21120 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:throughput_cache` | 0.257 | 307599 | 21120 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.263 | 275538 | 21120 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.398 | 55570 | 21120 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 50784 | 21120 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `large_burst:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.250 | 398423 | 21632 | `throughput_cache` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:cross_thread` | 0.250 | 398415 | 21632 | `cross_thread` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:balanced` | 0.258 | 341362 | 21632 | `balanced` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:fragmentation_stable` | 0.259 | 333917 | 21632 | `fragmentation_stable` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:deterministic_latency` | 0.260 | 327570 | 21632 | `deterministic_latency` | 0 | 246.58 | 0.000 | 0.940 |
| `fixed:large_object` | 0.416 | 59415 | 21632 | `large_object` | 0 | 56.51 | 0.000 | 1.832 |
| `fixed:compact_rss` | 0.490 | 57822 | 21632 | `compact_rss` | 0 | 109.59 | 0.000 | 1.811 |

## `latency_loop:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.058 | 6658482 | 14208 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.237 | 6808608 | 14208 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.284 | 6554960 | 14080 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.309 | 6376137 | 14080 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.364 | 6257815 | 14208 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.416 | 5932804 | 14208 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.553 | 5212615 | 14208 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 8998859 | 14592 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.265 | 7008621 | 14592 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.310 | 8036755 | 14592 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.351 | 8117942 | 14592 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.362 | 8033206 | 14592 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.368 | 7985282 | 14592 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.396 | 7778596 | 14592 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.250 | 8115131 | 14848 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.253 | 8100946 | 14848 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.283 | 7941236 | 14848 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.303 | 6883860 | 14976 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.313 | 7535478 | 14848 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.341 | 7645210 | 14848 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.498 | 6951749 | 14848 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.122 | 6799237 | 15488 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 7848185 | 15360 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.270 | 7396690 | 15488 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.312 | 7046510 | 15360 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.384 | 6667247 | 15488 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.411 | 6052271 | 15360 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.553 | 5272076 | 15488 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.093 | 5509782 | 15872 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.239 | 5830076 | 15744 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 6961214 | 15744 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.257 | 6737438 | 15744 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.257 | 6731515 | 15744 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.321 | 5188999 | 15744 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.515 | 3059303 | 15744 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.112 | 6802771 | 16256 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 7240628 | 16128 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.311 | 7100194 | 16256 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.403 | 6582449 | 16256 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.412 | 6346831 | 16128 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.502 | 5884164 | 16256 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.520 | 6025376 | 16256 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.146 | 5833556 | 16640 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 7002479 | 16512 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.320 | 6618540 | 16640 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.322 | 6174462 | 16512 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.326 | 5665024 | 16512 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.329 | 6508498 | 16640 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.515 | 4691208 | 16512 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.038 | 7383191 | 17024 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.313 | 6792302 | 17024 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.343 | 6945848 | 17024 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.367 | 6769779 | 17024 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.442 | 6052661 | 16896 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.514 | 5864588 | 17024 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.553 | 5663423 | 17024 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.177 | 7588380 | 17408 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 8170465 | 17408 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.253 | 8159525 | 17408 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.397 | 7524776 | 17408 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.400 | 7672339 | 17408 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.420 | 7609341 | 17408 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.515 | 7329774 | 17408 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.056 | 8113715 | 17792 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.240 | 8343953 | 17792 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 9076916 | 17792 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.280 | 8542750 | 17792 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.289 | 8390157 | 17792 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.387 | 7040396 | 17792 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.515 | 5821986 | 17792 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.227 | 7980609 | 18176 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 8607969 | 18176 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.265 | 4924709 | 18176 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.305 | 7460867 | 18176 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.310 | 7362800 | 18176 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.316 | 7248808 | 18176 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.390 | 6175977 | 18176 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 7780261 | 18432 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.265 | 6216893 | 18432 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.281 | 7559353 | 18432 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.289 | 7166074 | 18432 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.324 | 7269821 | 18432 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.382 | 6915760 | 18432 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.393 | 6850401 | 18432 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.062 | 6380604 | 18816 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.249 | 6199821 | 18688 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 6552008 | 18688 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.315 | 6361021 | 18816 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.333 | 6239569 | 18816 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.419 | 5502594 | 18688 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.515 | 5045346 | 18688 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.197 | 6783984 | 19200 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 8027836 | 19200 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.293 | 7380949 | 19200 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.330 | 7476114 | 19200 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.470 | 6666111 | 19200 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.499 | 6523284 | 19200 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.515 | 6444586 | 19200 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.198 | 7863974 | 19584 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 9346602 | 19584 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.298 | 8935736 | 19584 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.347 | 8557942 | 19584 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.389 | 8254298 | 19584 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.436 | 7623504 | 19584 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:balanced` | 0.515 | 7465769 | 19584 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:compact_rss` | 0.244 | 8180926 | 19968 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 8554179 | 19968 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:large_object` | 0.265 | 6759037 | 19968 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.296 | 8176792 | 19968 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.310 | 8066068 | 19968 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.365 | 7668640 | 19968 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.509 | 6791785 | 19968 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.041 | 7906234 | 20480 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 8327700 | 20480 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.258 | 8240560 | 20480 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.294 | 7399193 | 20480 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.322 | 7606087 | 20480 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.334 | 7503087 | 20480 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.515 | 6176336 | 20480 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.121 | 6489803 | 20992 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.199 | 7914769 | 20992 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.283 | 7466693 | 20992 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.324 | 6976103 | 20992 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.420 | 6054429 | 20992 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.470 | 5664739 | 20992 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.515 | 5350402 | 20992 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.250 | 8393481 | 21248 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.264 | 8203856 | 21248 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.279 | 7415816 | 21248 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:large_object` | 0.303 | 5847712 | 21376 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.377 | 7328662 | 21376 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.389 | 6834094 | 21248 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.424 | 6526593 | 21248 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `latency_loop:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:large_object` | 0.040 | 7854850 | 21760 | `large_object` | 0 | 56.89 | 0.000 | 0.001 |
| `fixed:compact_rss` | 0.244 | 7807453 | 21760 | `compact_rss` | 0 | 110.33 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 8204914 | 21760 | `deterministic_latency` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.257 | 8140030 | 21760 | `cross_thread` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:balanced` | 0.369 | 7245571 | 21760 | `balanced` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.426 | 6862691 | 21760 | `throughput_cache` | 0 | 124.12 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.515 | 6334192 | 21760 | `fragmentation_stable` | 0 | 124.12 | 0.000 | 0.001 |

## `remote_queue:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.065 | 532487 | 45824 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.069 | 545590 | 45696 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.105 | 581451 | 46080 | `deterministic_latency` | 0 | 2090.54 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.154 | 598822 | 47616 | `cross_thread` | 0 | 2795.49 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.290 | 765526 | 48256 | `throughput_cache` | 0 | 6453.94 | 0.186 | 0.025 |
| `fixed:large_object` | 0.406 | 153253 | 45696 | `large_object` | 0 | 62.29 | 0.186 | 0.273 |
| `fixed:compact_rss` | 0.427 | 144747 | 45824 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |

## `remote_queue:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.060 | 508299 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.068 | 501210 | 46080 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.112 | 483054 | 46208 | `deterministic_latency` | 0 | 2187.77 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.136 | 569991 | 47488 | `cross_thread` | 0 | 2734.72 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.291 | 669728 | 48896 | `throughput_cache` | 0 | 6526.86 | 0.186 | 0.026 |
| `fixed:large_object` | 0.389 | 161948 | 46208 | `large_object` | 0 | 62.29 | 0.186 | 0.273 |
| `fixed:compact_rss` | 0.427 | 144217 | 46208 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |

## `remote_queue:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.057 | 632729 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.187 | 0.022 |
| `fixed:balanced` | 0.065 | 691444 | 46464 | `balanced` | 0 | 1227.59 | 0.187 | 0.022 |
| `fixed:deterministic_latency` | 0.109 | 690313 | 46592 | `deterministic_latency` | 0 | 2272.86 | 0.187 | 0.023 |
| `fixed:cross_thread` | 0.152 | 695119 | 47872 | `cross_thread` | 0 | 2807.64 | 0.187 | 0.025 |
| `fixed:throughput_cache` | 0.291 | 850799 | 48384 | `throughput_cache` | 0 | 6429.63 | 0.187 | 0.026 |
| `fixed:compact_rss` | 0.365 | 186292 | 46336 | `compact_rss` | 0 | 109.39 | 0.187 | 0.272 |
| `fixed:large_object` | 0.424 | 149204 | 46080 | `large_object` | 0 | 62.29 | 0.187 | 0.274 |

## `remote_queue:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.060 | 579770 | 46592 | `fragmentation_stable` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:balanced` | 0.060 | 619033 | 46336 | `balanced` | 0 | 1239.74 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.100 | 622765 | 46464 | `deterministic_latency` | 0 | 2212.08 | 0.183 | 0.022 |
| `fixed:cross_thread` | 0.146 | 635508 | 47872 | `cross_thread` | 0 | 2795.49 | 0.183 | 0.025 |
| `fixed:throughput_cache` | 0.291 | 712862 | 48640 | `throughput_cache` | 0 | 6368.85 | 0.183 | 0.026 |
| `fixed:large_object` | 0.424 | 158696 | 46208 | `large_object` | 0 | 62.29 | 0.183 | 0.274 |
| `fixed:compact_rss` | 0.428 | 159239 | 46464 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |

## `remote_queue:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.055 | 654042 | 45824 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.067 | 667123 | 45952 | `balanced` | 0 | 1239.74 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.102 | 707357 | 46080 | `deterministic_latency` | 0 | 2199.93 | 0.186 | 0.023 |
| `fixed:cross_thread` | 0.158 | 610923 | 47104 | `cross_thread` | 0 | 2783.34 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.290 | 839342 | 47872 | `throughput_cache` | 0 | 6466.09 | 0.186 | 0.025 |
| `fixed:compact_rss` | 0.424 | 176680 | 45824 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:large_object` | 0.427 | 176147 | 45952 | `large_object` | 0 | 62.29 | 0.186 | 0.273 |

## `remote_queue:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.059 | 530329 | 45696 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.063 | 594494 | 45824 | `balanced` | 0 | 1227.59 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.096 | 641521 | 45952 | `deterministic_latency` | 0 | 2127.00 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.155 | 548957 | 47232 | `cross_thread` | 0 | 2710.41 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.290 | 759605 | 47744 | `throughput_cache` | 0 | 6466.09 | 0.185 | 0.025 |
| `fixed:compact_rss` | 0.403 | 143475 | 45952 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.427 | 131122 | 45824 | `large_object` | 0 | 62.29 | 0.185 | 0.273 |

## `remote_queue:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.065 | 537307 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.182 | 0.022 |
| `fixed:balanced` | 0.065 | 595210 | 46208 | `balanced` | 0 | 1203.28 | 0.182 | 0.022 |
| `fixed:deterministic_latency` | 0.111 | 602038 | 46464 | `deterministic_latency` | 0 | 2272.86 | 0.182 | 0.023 |
| `fixed:cross_thread` | 0.167 | 509784 | 47616 | `cross_thread` | 0 | 2795.49 | 0.182 | 0.024 |
| `fixed:throughput_cache` | 0.291 | 764334 | 48256 | `throughput_cache` | 0 | 6429.63 | 0.182 | 0.026 |
| `fixed:large_object` | 0.401 | 158167 | 46080 | `large_object` | 0 | 62.29 | 0.182 | 0.273 |
| `fixed:compact_rss` | 0.430 | 146988 | 46336 | `compact_rss` | 0 | 109.39 | 0.182 | 0.272 |

## `remote_queue:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.064 | 529436 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.067 | 569792 | 46336 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.100 | 623509 | 46336 | `deterministic_latency` | 0 | 2212.08 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.155 | 547736 | 47744 | `cross_thread` | 0 | 2771.18 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.291 | 728622 | 48640 | `throughput_cache` | 0 | 6393.16 | 0.185 | 0.026 |
| `fixed:large_object` | 0.424 | 143659 | 46080 | `large_object` | 0 | 62.29 | 0.185 | 0.273 |
| `fixed:compact_rss` | 0.425 | 143533 | 46080 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |

## `remote_queue:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.060 | 646911 | 46080 | `balanced` | 0 | 1239.74 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.063 | 558053 | 46080 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.100 | 653044 | 46208 | `deterministic_latency` | 0 | 2224.24 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.173 | 526015 | 47616 | `cross_thread` | 0 | 2807.64 | 0.186 | 0.025 |
| `fixed:throughput_cache` | 0.290 | 777224 | 48000 | `throughput_cache` | 0 | 6429.63 | 0.186 | 0.025 |
| `fixed:compact_rss` | 0.417 | 164703 | 46208 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:large_object` | 0.424 | 159092 | 46080 | `large_object` | 0 | 62.29 | 0.186 | 0.273 |

## `remote_queue:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.059 | 674758 | 46464 | `balanced` | 0 | 1191.12 | 0.185 | 0.022 |
| `fixed:fragmentation_stable` | 0.065 | 566856 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.100 | 661361 | 46592 | `deterministic_latency` | 0 | 2139.16 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.146 | 661053 | 47872 | `cross_thread` | 0 | 2734.72 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.290 | 796869 | 48640 | `throughput_cache` | 0 | 6490.40 | 0.185 | 0.025 |
| `fixed:compact_rss` | 0.426 | 173072 | 46592 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.426 | 171236 | 46464 | `large_object` | 0 | 62.29 | 0.185 | 0.273 |

## `remote_queue:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.059 | 710460 | 46080 | `balanced` | 0 | 1166.81 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.060 | 629766 | 45952 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.103 | 719018 | 46336 | `deterministic_latency` | 0 | 2224.24 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.143 | 690565 | 47616 | `cross_thread` | 0 | 2686.10 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.291 | 833844 | 48512 | `throughput_cache` | 0 | 6490.40 | 0.186 | 0.026 |
| `fixed:large_object` | 0.424 | 176324 | 46080 | `large_object` | 0 | 62.29 | 0.186 | 0.273 |
| `fixed:compact_rss` | 0.425 | 174329 | 45824 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |

## `remote_queue:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.055 | 541318 | 46080 | `fragmentation_stable` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:balanced` | 0.058 | 602018 | 46208 | `balanced` | 0 | 1203.28 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.104 | 559163 | 46464 | `deterministic_latency` | 0 | 2102.69 | 0.183 | 0.022 |
| `fixed:cross_thread` | 0.144 | 582845 | 47616 | `cross_thread` | 0 | 2686.10 | 0.183 | 0.024 |
| `fixed:throughput_cache` | 0.290 | 691538 | 48384 | `throughput_cache` | 0 | 6320.24 | 0.183 | 0.025 |
| `fixed:compact_rss` | 0.406 | 147577 | 46208 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |
| `fixed:large_object` | 0.428 | 137742 | 46336 | `large_object` | 0 | 62.29 | 0.183 | 0.273 |

## `remote_queue:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.060 | 488008 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.063 | 569611 | 46592 | `balanced` | 0 | 1215.43 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.097 | 574950 | 46464 | `deterministic_latency` | 0 | 2151.31 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.133 | 582957 | 47488 | `cross_thread` | 0 | 2686.10 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.291 | 704468 | 48896 | `throughput_cache` | 0 | 6466.09 | 0.185 | 0.026 |
| `fixed:compact_rss` | 0.356 | 150386 | 46464 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |
| `fixed:large_object` | 0.424 | 115693 | 46208 | `large_object` | 0 | 62.29 | 0.185 | 0.273 |

## `remote_queue:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.064 | 484057 | 46592 | `balanced` | 0 | 1191.12 | 0.184 | 0.022 |
| `fixed:fragmentation_stable` | 0.069 | 429503 | 46720 | `fragmentation_stable` | 0 | 972.34 | 0.184 | 0.022 |
| `fixed:deterministic_latency` | 0.106 | 502520 | 46976 | `deterministic_latency` | 0 | 2163.47 | 0.184 | 0.023 |
| `fixed:cross_thread` | 0.140 | 503910 | 47872 | `cross_thread` | 0 | 2673.95 | 0.184 | 0.024 |
| `fixed:throughput_cache` | 0.291 | 613427 | 49024 | `throughput_cache` | 0 | 6405.32 | 0.184 | 0.026 |
| `fixed:large_object` | 0.423 | 132949 | 46720 | `large_object` | 0 | 62.29 | 0.184 | 0.274 |
| `fixed:compact_rss` | 0.427 | 131878 | 46720 | `compact_rss` | 0 | 109.39 | 0.184 | 0.272 |

## `remote_queue:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.054 | 600558 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.054 | 665259 | 46336 | `balanced` | 0 | 1178.97 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.097 | 644569 | 46592 | `deterministic_latency` | 0 | 2102.69 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.146 | 650050 | 48128 | `cross_thread` | 0 | 2783.34 | 0.185 | 0.025 |
| `fixed:throughput_cache` | 0.291 | 740275 | 48896 | `throughput_cache` | 0 | 6453.94 | 0.185 | 0.026 |
| `fixed:large_object` | 0.404 | 178166 | 46336 | `large_object` | 0 | 62.29 | 0.185 | 0.273 |
| `fixed:compact_rss` | 0.425 | 166697 | 46208 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |

## `remote_queue:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.065 | 625659 | 46208 | `balanced` | 0 | 1191.12 | 0.183 | 0.022 |
| `fixed:fragmentation_stable` | 0.068 | 555771 | 46336 | `fragmentation_stable` | 0 | 972.34 | 0.183 | 0.022 |
| `fixed:deterministic_latency` | 0.102 | 630092 | 46464 | `deterministic_latency` | 0 | 2078.39 | 0.183 | 0.022 |
| `fixed:cross_thread` | 0.157 | 596748 | 48000 | `cross_thread` | 0 | 2795.49 | 0.183 | 0.025 |
| `fixed:throughput_cache` | 0.291 | 849415 | 49024 | `throughput_cache` | 0 | 6502.55 | 0.183 | 0.026 |
| `fixed:large_object` | 0.400 | 168142 | 46464 | `large_object` | 0 | 62.29 | 0.183 | 0.274 |
| `fixed:compact_rss` | 0.428 | 153860 | 46464 | `compact_rss` | 0 | 109.39 | 0.183 | 0.272 |

## `remote_queue:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.058 | 596206 | 45824 | `fragmentation_stable` | 0 | 972.34 | 0.185 | 0.022 |
| `fixed:balanced` | 0.059 | 630264 | 45696 | `balanced` | 0 | 1178.97 | 0.185 | 0.022 |
| `fixed:deterministic_latency` | 0.096 | 687608 | 46080 | `deterministic_latency` | 0 | 2187.77 | 0.185 | 0.022 |
| `fixed:cross_thread` | 0.140 | 644012 | 47232 | `cross_thread` | 0 | 2759.03 | 0.185 | 0.024 |
| `fixed:throughput_cache` | 0.291 | 766251 | 48640 | `throughput_cache` | 0 | 6478.24 | 0.185 | 0.026 |
| `fixed:large_object` | 0.407 | 173389 | 45824 | `large_object` | 0 | 62.29 | 0.185 | 0.274 |
| `fixed:compact_rss` | 0.428 | 163772 | 45952 | `compact_rss` | 0 | 109.39 | 0.185 | 0.272 |

## `remote_queue:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.064 | 636733 | 46080 | `balanced` | 0 | 1191.12 | 0.186 | 0.022 |
| `fixed:fragmentation_stable` | 0.070 | 561151 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.106 | 659704 | 46336 | `deterministic_latency` | 0 | 2212.08 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.150 | 655757 | 47744 | `cross_thread` | 0 | 2625.33 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.290 | 810168 | 48128 | `throughput_cache` | 0 | 6417.47 | 0.186 | 0.025 |
| `fixed:compact_rss` | 0.421 | 174115 | 46080 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:large_object` | 0.424 | 171474 | 46060 | `large_object` | 0 | 62.29 | 0.186 | 0.273 |

## `remote_queue:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.053 | 587873 | 46080 | `fragmentation_stable` | 0 | 972.34 | 0.186 | 0.022 |
| `fixed:balanced` | 0.054 | 618121 | 45952 | `balanced` | 0 | 1166.81 | 0.186 | 0.022 |
| `fixed:deterministic_latency` | 0.098 | 599171 | 46080 | `deterministic_latency` | 0 | 2163.47 | 0.186 | 0.022 |
| `fixed:cross_thread` | 0.125 | 704519 | 47616 | `cross_thread` | 0 | 2637.48 | 0.186 | 0.024 |
| `fixed:throughput_cache` | 0.293 | 688649 | 48640 | `throughput_cache` | 0 | 6514.71 | 0.186 | 0.026 |
| `fixed:compact_rss` | 0.419 | 167870 | 45952 | `compact_rss` | 0 | 109.39 | 0.186 | 0.272 |
| `fixed:large_object` | 0.426 | 165163 | 46080 | `large_object` | 0 | 62.29 | 0.186 | 0.274 |

## `remote_queue:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.058 | 576286 | 46208 | `fragmentation_stable` | 0 | 972.34 | 0.184 | 0.022 |
| `fixed:balanced` | 0.064 | 594843 | 46208 | `balanced` | 0 | 1203.28 | 0.184 | 0.022 |
| `fixed:deterministic_latency` | 0.100 | 644098 | 46336 | `deterministic_latency` | 0 | 2224.24 | 0.184 | 0.023 |
| `fixed:cross_thread` | 0.153 | 604691 | 47872 | `cross_thread` | 0 | 2746.87 | 0.184 | 0.025 |
| `fixed:throughput_cache` | 0.291 | 766923 | 48512 | `throughput_cache` | 0 | 6466.09 | 0.184 | 0.026 |
| `fixed:large_object` | 0.394 | 175402 | 46336 | `large_object` | 0 | 62.29 | 0.184 | 0.273 |
| `fixed:compact_rss` | 0.425 | 158204 | 46208 | `compact_rss` | 0 | 109.39 | 0.184 | 0.272 |

## `rss_peak_release:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1546373 | 14080 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1722609 | 14080 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.251 | 1583306 | 14080 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.251 | 1491616 | 14080 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.252 | 1324022 | 14080 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.394 | 40205 | 14080 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.397 | 44713 | 14080 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.166 | 1678093 | 14464 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 2090321 | 14464 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 2097134 | 14464 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.251 | 1882720 | 14464 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.252 | 1553033 | 14464 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.370 | 57226 | 14464 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.424 | 52092 | 14464 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.202 | 2027133 | 14848 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.249 | 1705095 | 14720 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 2138034 | 14720 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 2112615 | 14720 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.289 | 1848846 | 14848 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.356 | 54908 | 14720 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.462 | 47181 | 14848 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.202 | 1831400 | 15360 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1826032 | 15232 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.253 | 1263878 | 15232 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.288 | 1797952 | 15360 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.289 | 1662198 | 15360 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.432 | 45867 | 15360 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.435 | 51038 | 15360 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.203 | 1824247 | 15744 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.249 | 1749363 | 15616 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 2168649 | 15616 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.251 | 2046561 | 15616 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.290 | 1782205 | 15744 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.391 | 53933 | 15616 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.462 | 53374 | 15744 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.203 | 1744434 | 16128 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.249 | 1611954 | 16000 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 1946412 | 16000 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 1928528 | 16000 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.291 | 1327217 | 16128 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.425 | 46431 | 16128 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.462 | 45142 | 16128 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.164 | 1671016 | 16384 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1653332 | 16384 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 1799166 | 16384 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 1723522 | 16384 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.251 | 1638624 | 16384 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.394 | 43864 | 16384 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.414 | 45530 | 16384 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1663993 | 16896 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1638612 | 16896 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 1805775 | 16896 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.250 | 1736787 | 16896 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.251 | 1517736 | 16896 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.371 | 46713 | 16896 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.424 | 42779 | 16896 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1797147 | 17280 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1752205 | 17280 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 1953626 | 17280 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.252 | 1614321 | 17280 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.253 | 1381488 | 17280 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.394 | 47900 | 17280 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.410 | 50497 | 17280 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1883837 | 17664 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 2066451 | 17664 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 2157669 | 17664 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.251 | 2006352 | 17664 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.251 | 1865099 | 17664 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.385 | 55059 | 17664 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.424 | 53140 | 17664 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1966042 | 18048 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1971949 | 18048 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.250 | 2157764 | 18048 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.251 | 2007375 | 18048 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.251 | 1927131 | 18048 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.394 | 53457 | 18048 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.418 | 54720 | 18048 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1551002 | 18304 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1556274 | 18304 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.250 | 1684439 | 18304 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.251 | 1427009 | 18304 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.253 | 1185740 | 18304 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.368 | 45637 | 18304 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.424 | 41268 | 18304 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.203 | 1552783 | 18688 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1611585 | 18560 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 1752477 | 18560 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.289 | 1692402 | 18688 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.290 | 1469635 | 18688 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.432 | 40818 | 18688 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.455 | 42012 | 18688 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1695392 | 19072 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1786592 | 19072 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 1882973 | 19072 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.253 | 1322760 | 19072 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.254 | 1206308 | 19072 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.387 | 50441 | 19072 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.424 | 49087 | 19072 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.164 | 1984548 | 19456 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1894013 | 19456 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 2002303 | 19456 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.251 | 1854014 | 19456 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.253 | 1447207 | 19456 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.391 | 54521 | 19456 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.424 | 53836 | 19456 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.166 | 1482942 | 19840 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.249 | 1526361 | 19840 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.250 | 1917919 | 19840 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.251 | 1773655 | 19840 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.251 | 1743734 | 19840 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.394 | 49689 | 19840 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.449 | 52183 | 19968 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.164 | 1913977 | 20352 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1925163 | 20352 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 2054571 | 20352 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 2007277 | 20352 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.250 | 1963126 | 20352 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.394 | 52673 | 20352 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.423 | 52910 | 20352 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1841146 | 20864 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 1839819 | 20864 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 2011520 | 20864 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.250 | 1992723 | 20864 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.251 | 1855940 | 20864 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.394 | 50563 | 20864 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.413 | 52789 | 20864 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.203 | 1795020 | 21248 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 2005523 | 21120 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.289 | 1846055 | 21248 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.290 | 1594992 | 21248 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.290 | 1558701 | 21248 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.416 | 47836 | 21248 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.462 | 44992 | 21248 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `rss_peak_release:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.165 | 1779712 | 21632 | `fragmentation_stable` | 0 | 3578.90 | 0.000 | 0.048 |
| `fixed:balanced` | 0.248 | 2058089 | 21632 | `balanced` | 0 | 5375.15 | 0.000 | 0.048 |
| `fixed:throughput_cache` | 0.250 | 2041688 | 21632 | `throughput_cache` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:deterministic_latency` | 0.250 | 2027241 | 21632 | `deterministic_latency` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:cross_thread` | 0.250 | 2015313 | 21632 | `cross_thread` | 0 | 5429.58 | 0.000 | 0.048 |
| `fixed:compact_rss` | 0.388 | 51885 | 21632 | `compact_rss` | 0 | 108.86 | 0.000 | 1.150 |
| `fixed:large_object` | 0.424 | 50665 | 21632 | `large_object` | 0 | 56.13 | 0.000 | 1.433 |

## `throughput_churn:seed10001` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.250 | 5299342 | 13824 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.251 | 4171498 | 13824 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.252 | 3250527 | 13824 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.289 | 4139955 | 13952 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.289 | 4029138 | 13952 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.440 | 50515 | 13952 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 46354 | 13824 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10002` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 6327412 | 14336 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 6234568 | 14336 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 6111622 | 14336 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.251 | 5146526 | 14336 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.251 | 4625192 | 14336 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.424 | 52438 | 14336 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.510 | 55035 | 14336 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10003` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.250 | 8454366 | 14720 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 8327074 | 14720 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 7703481 | 14720 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.251 | 6943933 | 14720 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.251 | 6764070 | 14720 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.424 | 61573 | 14720 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 61553 | 14720 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10004` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 7791314 | 14976 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 7683085 | 14976 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 7019889 | 14976 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 6665261 | 14976 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.251 | 6292854 | 14976 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.462 | 57358 | 15104 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.515 | 58912 | 14976 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10005` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 7977982 | 15488 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 7577468 | 15488 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 7353741 | 15488 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.250 | 7048697 | 15488 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 6933672 | 15488 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.422 | 60788 | 15488 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 60246 | 15488 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10006` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.250 | 6505512 | 15872 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 6301099 | 15872 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 5784096 | 15872 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 5750178 | 15872 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.288 | 6370274 | 16000 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.461 | 51477 | 16000 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 51160 | 15872 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10007` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 6391454 | 16256 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.250 | 5898560 | 16256 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 5816554 | 16256 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 5635953 | 16256 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.253 | 2945021 | 16256 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.418 | 51452 | 16256 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 50233 | 16256 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10008` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:fragmentation_stable` | 0.250 | 6848971 | 16640 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 6152661 | 16640 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 6128383 | 16640 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 5963490 | 16640 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.251 | 5347693 | 16640 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.462 | 50178 | 16768 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.505 | 53664 | 16640 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10009` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.250 | 7469715 | 17024 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 7137015 | 17024 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.250 | 6871562 | 17024 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.251 | 5903844 | 17024 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.251 | 5124995 | 17024 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.424 | 53607 | 17024 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.513 | 55591 | 17024 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10010` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 7874611 | 17536 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.250 | 7429238 | 17536 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 7186493 | 17536 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.251 | 6258255 | 17536 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.251 | 5724035 | 17536 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.424 | 58205 | 17536 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.504 | 62480 | 17536 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10011` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.250 | 7452966 | 17792 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.288 | 8505121 | 17920 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.288 | 8044311 | 17920 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.289 | 7621408 | 17920 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.289 | 6813498 | 17920 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.453 | 63009 | 17920 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.561 | 60844 | 17920 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10012` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 7121282 | 18176 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 6971445 | 18176 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.250 | 6622910 | 18176 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.251 | 5103311 | 18176 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.252 | 3935849 | 18176 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.417 | 52983 | 18176 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 51629 | 18176 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10013` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.250 | 6416358 | 18432 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 6387243 | 18432 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.251 | 4659416 | 18432 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.288 | 6329525 | 18560 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.289 | 5562245 | 18560 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.454 | 50644 | 18560 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 48991 | 18432 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10014` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.250 | 6027741 | 18816 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 5702630 | 18816 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.251 | 4381977 | 18816 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.253 | 2729250 | 18816 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.290 | 3180375 | 18944 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.447 | 47744 | 18944 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 44935 | 18816 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10015` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 7988141 | 19328 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 7685587 | 19328 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 7078802 | 19328 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.251 | 6728480 | 19328 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.251 | 6282169 | 19328 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.413 | 62559 | 19328 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 60027 | 19328 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10016` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:throughput_cache` | 0.250 | 8400024 | 19712 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 7561258 | 19712 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.288 | 7861539 | 19840 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.288 | 7850956 | 19840 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.289 | 7198521 | 19840 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.462 | 59317 | 19840 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.551 | 61611 | 19840 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10017` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:deterministic_latency` | 0.250 | 7650085 | 20096 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 7365887 | 20096 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.250 | 6609779 | 20096 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.251 | 5986342 | 20096 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.251 | 5472368 | 20096 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.419 | 59897 | 20096 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.522 | 58726 | 20096 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10018` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 8009382 | 20608 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.250 | 7960508 | 20608 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 7112483 | 20608 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.250 | 7062581 | 20608 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.251 | 5862200 | 20608 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.424 | 59358 | 20608 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.516 | 60879 | 20608 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10019` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:balanced` | 0.250 | 6427841 | 20992 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.288 | 6898643 | 21120 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.288 | 6588543 | 21120 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.289 | 6252774 | 21120 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:cross_thread` | 0.289 | 6115259 | 21120 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.462 | 58159 | 21120 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.560 | 58179 | 21120 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

## `throughput_churn:seed10020` Details

| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |
|---|---:|---:|---:|---|---:|---:|---:|---:|
| `fixed:cross_thread` | 0.250 | 8168196 | 21376 | `cross_thread` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:throughput_cache` | 0.250 | 7929700 | 21376 | `throughput_cache` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:fragmentation_stable` | 0.250 | 7619837 | 21376 | `fragmentation_stable` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:balanced` | 0.250 | 7100786 | 21376 | `balanced` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:deterministic_latency` | 0.251 | 5617105 | 21376 | `deterministic_latency` | 0 | 190.51 | 0.000 | 0.001 |
| `fixed:large_object` | 0.424 | 59398 | 21376 | `large_object` | 0 | 56.13 | 0.000 | 1.000 |
| `fixed:compact_rss` | 0.507 | 62934 | 21376 | `compact_rss` | 0 | 108.86 | 0.000 | 1.000 |

