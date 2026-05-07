# Adaptive Workload Visualization

`bench_runner` can optionally serve a lightweight local web UI while a generated
workload runs:

```bash
./build/bench_runner --strategy adaptive --bench generated_workload \
  --workload-template adaptive_mix --iters 200000 --slots 4096 --threads 4 \
  --telemetry-port 8080
```

Open:

```text
http://127.0.0.1:8080/
```

The UI also works for non-adaptive strategies:

```bash
./build/bench_runner --strategy ptmalloc --bench generated_workload \
  --workload-template adaptive_mix --telemetry-port 8080
```

When `--telemetry-port` is enabled and output is not JSON, `bench_runner` keeps
the local UI server alive for five minutes after the workload finishes. This
prevents the browser from showing "connection refused" immediately after a short
workload completes. Use `--telemetry-hold-ms 0` to return to one-shot benchmark
behavior, or pass a larger value for longer inspection:

```bash
./build/bench_runner --strategy adaptive --bench generated_workload \
  --workload-template adaptive_mix --telemetry-port 8080 --telemetry-hold-ms 600000
```

For non-adaptive strategies, the page shows workload phase, ops/sec, live-set
estimates, requested bytes, remote-free count, and peak RSS. For adaptive runs,
it also shows current mode, previous mode, mode switches, mapped/live ratio,
remote-free ratio, large-object ratio, fragmentation estimate, slow-path ratio,
and safety counters.

## Isolation Rules

Visualization is tool-side only.

- It is disabled by default.
- No socket, server thread, HTML, JSON formatting, or polling exists unless
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
| `/` | Single-file HTML/CSS/JavaScript UI embedded in `bench_runner` |
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
    "previous_mode": "throughput_cache",
    "mode_switches": 2,
    "mapped_live_ratio": 10.5,
    "remote_free_ratio": 0.25,
    "large_bytes_ratio": 0.70,
    "fragmentation_estimate": 0.04,
    "slow_path_ratio": 0.12
  }
}
```

The `adaptive` object is omitted for non-adaptive strategies.
