# Adaptive Workload Visualization

`bench_runner` can optionally serve a lightweight local web UI while a generated
workload runs:

```bash
./build/bench_runner --strategy adaptive --bench generated_workload \
  --workload-template adaptive_mix \
  --workload-realtime --phase-ms 10000 --target-ops-per-sec 50000 \
  --telemetry-port 8080
```

Open:

```text
http://127.0.0.1:8080/
```

The UI also works for non-adaptive strategies:

```bash
./build/bench_runner --strategy ptmalloc --bench generated_workload \
  --workload-template adaptive_mix --workload-realtime --phase-ms 10000 \
  --telemetry-port 8080
```

Without `--workload-realtime`, generated workloads run as fast benchmarks and
may finish before the browser shows useful phase changes. Use realtime mode for
watching dynamic behavior.

When `--telemetry-port` is enabled and output is not JSON, `bench_runner` keeps
the local UI server alive for five minutes after the workload finishes. This
prevents the browser from showing "connection refused" immediately after a short
workload completes. Use `--telemetry-hold-ms 0` to return to one-shot benchmark
behavior, or pass a larger value for longer inspection:

```bash
./build/bench_runner --strategy adaptive --bench generated_workload \
  --workload-template adaptive_mix --workload-realtime --phase-ms 10000 \
  --target-ops-per-sec 50000 --telemetry-port 8080 --telemetry-hold-ms 600000
```

For non-adaptive strategies, the page shows workload phase, ops/sec, live-set
estimates, requested bytes, remote-free count, and peak RSS. For adaptive runs,
it also renders allocator internals: the facade request flow, `AdaptiveHeader`
and ownership routing, active mode policy intent, release decision, size-class
pages, spans, direct maps, TLS/cache reuse, remote-free queue pressure,
purge/unmap activity, debug/quarantine safety signals, and the selector
telemetry loop. Adaptive selector decisions are read from the selector event
ring buffer. The Web snapshot does not call workload feature extraction; it
only renders events already recorded at selector window boundaries.

The page uses a SpaceX-style black/white telemetry dashboard treatment: high
contrast text, thin borders, compact status pills, a phase progress track,
mission-control metric cards with inline SVG icons, grouped workload/adaptive
tables, and lightweight canvas charts for ops/sec, live memory, mapped memory,
and mapped/live trends.

The hero panel includes a SpaceX engine-ignition-style mode rail: all eight
adaptive modes are listed in order, the active mode is marked by a white dot,
the previous mode keeps an amber outline, and a mode switch triggers a short
ignition pulse on the active dot.

The internal-structure panel is not a separate instrumentation path. It is a
visual projection of fields already exposed by `AdaptiveStatsSnapshot` and the
selector event ring buffer:

- storage arrays show size-class page, span, and direct-map allocation share;
- pool hit/miss arrays show cache and central-pool reuse pressure;
- release counters show empty page/span reclaim and unmapped bytes;
- safety counters show invalid free, double free, and header-corruption events;
- selector events show the mode candidate, backend, confidence, and reason.

The UI is implemented as static assets under `tools/web_viewer/`:

- `index.html`
- `app.css`
- `app.js`

`TelemetryServer` serves those files and `/snapshot`. The allocator library does
not load frontend assets, and no frontend code is embedded in the allocator hot
path.

The mode timeline explains decisions in plain terms, for example
`balanced -> large_object` with reason `model_cost` and the feature values that
made the generated model prefer the candidate. `large_bytes_ratio` is a
request-size signal for allocations above 256 KiB, not a DirectMap storage
counter, so the UI does not report a large-object phase simply because a mode
used direct mapping internally.

Runtime charts are split by signal instead of drawing every metric on one shared
normalized axis. Each signal has its own min/max scale, current value label, and
unit. The frontend applies a small EMA smoothing step so allocator bursts remain
readable without hiding phase changes.

## Isolation Rules

Visualization is tool-side only.

- It is disabled by default.
- No socket, server thread, static asset serving, JSON formatting, or polling exists unless
  `--telemetry-port` is passed.
- The allocator hot path does not perform I/O.
- The web server lives in `bench_runner`, not in the allocator library.
- The browser polls `/snapshot`; allocator mode selection does not read from
  the viewer.
- Snapshot reads use existing benchmark counters plus adaptive relaxed-atomic
  snapshots when the selected strategy is `adaptive`.

This keeps allocator behavior comparable with and without visualization. The
viewer is intended for human inspection of changing phases and adaptive mode
switching, not for high-precision performance measurement.

## Endpoints

| Endpoint | Purpose |
|---|---|
| `/` | Static `tools/web_viewer/index.html` |
| `/app.css` | Static viewer stylesheet |
| `/app.js` | Static viewer script |
| `/snapshot` | JSON snapshot for the current workload and optional adaptive telemetry |

The server binds only to `127.0.0.1`.

## Snapshot Shape

`/snapshot` returns:

```json
{
  "workload": {
    "strategy": "adaptive",
    "benchmark": "generated_workload",
    "template": "adaptive_mix",
    "phase": "large_burst",
    "phase_index": 3,
    "phase_count": 6,
    "running": true,
    "phase_elapsed_ms": 4200,
    "phase_duration_ms": 10000,
    "phase_progress": 0.42,
    "ops_per_sec": 250000,
    "allocs": 10000,
    "frees": 9800,
    "reallocs": 5000,
    "remote_frees": 3000,
    "requested_bytes": 500000000,
    "live_objects": 200,
    "live_bytes": 4000000,
    "peak_live_bytes": 26000000,
    "peak_rss_kb": 45000
  },
  "adaptive": {
    "current_mode": "large_object",
    "active_mode": "large_object",
    "previous_mode": "throughput_cache",
    "mode_switches": 2,
    "retired_mode_count": 1,
    "mapped_bytes": 64000000,
    "live_bytes": 4000000,
    "mapped_live_ratio": 10.5,
    "remote_free_ratio": 0.25,
    "large_bytes_ratio": 0.70,
    "fragmentation_estimate": 0.04,
    "slow_path_ratio": 0.12,
    "empty_pages": 4,
    "empty_spans": 1,
    "released_pages": 20,
    "released_spans": 5,
    "release_unmapped_bytes": 32000000,
    "mode_alloc_count": [100, 200, 0, 20, 40, 10, 80, 0],
    "mode_free_count": [90, 180, 0, 18, 35, 10, 70, 0],
    "mode_live_bytes": [0, 4096, 0, 0, 8192, 0, 3000000, 0],
    "mode_mapped_bytes": [65536, 1048576, 0, 0, 2097152, 0, 32000000, 0],
    "storage_allocs": [200, 120, 80],
    "storage_frees": [190, 110, 70],
    "storage_requested_bytes": [128000, 4096000, 50000000],
    "storage_usable_bytes": [160000, 4300000, 52000000],
    "pool_hits": [150, 50, 0],
    "pool_misses": [50, 70, 80]
  },
  "selector_last_window": {
    "selector_backend": "model",
    "reason": "model_cost",
    "model_candidate": "large_object",
    "model_confidence": 0.42,
    "large_bytes_ratio": 0.91,
    "remote_free_ratio": 0.0,
    "mapped_live_ratio": 1.3,
    "switched": true
  },
  "selector_events": [
    {
      "current_mode": "balanced",
      "candidate_mode": "large_object",
      "switched": true,
      "selector_backend": "model",
      "model_candidate": "large_object",
      "model_confidence": 0.42,
      "reason": "model_cost"
    }
  ]
}
```

The `adaptive` object is omitted for non-adaptive strategies. In this payload,
`large_bytes_ratio` means the fraction of requested bytes in the true
streaming-large request buckets above 256 KiB.
