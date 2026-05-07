# Adaptive Workload Generator

The generated workload is a benchmark workload in `bench_runner`, parallel to
`same_size_64`, `fragmentation`, `phase_changing`, and the other built-in
benchmarks. It is not adaptive-specific execution logic: it drives allocations
only through `StrategyDescriptor::vtable`, so every strategy can run the exact
same generated workload.

Example:

```bash
./build/bench_runner --strategy hybrid --bench generated_workload --workload-template adaptive_mix --json
./build/bench_runner --strategy ptmalloc --bench generated_workload --workload-template adaptive_mix --json
./build/bench_runner --strategy tcmalloc_like --bench generated_workload --workload-template adaptive_mix --json
./build/bench_runner --strategy jemalloc_like --bench generated_workload --workload-template adaptive_mix --json
./build/bench_runner --strategy mimalloc_like --bench generated_workload --workload-template adaptive_mix --json
./build/bench_runner --strategy adaptive --bench generated_workload --workload-template adaptive_mix --json
./build/bench_runner --strategy libc --bench generated_workload --workload-template adaptive_mix --json
```

## Design

The generator is deliberately located in the benchmark/tool layer:

```text
generated_workload
  -> StrategyDescriptor vtable
  -> common generated workload metrics
  -> optional adaptive snapshot when strategy == adaptive
```

The generator never calls adaptive-only allocation APIs to produce load. For
adaptive runs, telemetry is observed after the fact through
`adaptive_stats_snapshot()`.

## Templates

Use `--workload-template NAME`.

| Template | Phases | Purpose |
|---|---|---|
| `adaptive_mix` | small churn, fragmentation drift, remote free, large burst, peak release, latency loop | Exercises multiple adaptive mode triggers in one run |
| `throughput_churn` | small churn, latency loop | Local short-lived allocation pressure |
| `remote_queue` | small churn, remote free | Producer/consumer and cross-thread free pressure |
| `large_burst` | small churn, large burst | Large object streaming pressure |
| `rss_peak_release` | peak release, small churn | Retained-memory pressure after a peak |
| `fragmentation_drift` | fragmentation drift | Mixed medium-size reallocation and waste pressure |
| `latency_loop` | latency loop | Stable repeated allocation/free loop |

Global benchmark knobs still apply:

- `--strategy`
- `--iters`
- `--slots`
- `--threads`
- `--seed`
- `--json`
- `--repeats`

## Benchmark Mode vs Realtime Mode

By default, `generated_workload` runs as a benchmark and completes as fast as
possible. This is the right mode for allocator comparisons.

For visualization, use realtime mode:

```bash
./build/bench_runner --strategy adaptive --bench generated_workload \
  --workload-template adaptive_mix \
  --workload-realtime --phase-ms 10000 --target-ops-per-sec 50000 \
  --telemetry-port 8080
```

Realtime options:

| Option | Meaning |
|---|---|
| `--workload-realtime` | Run phases by wall-clock time instead of completing as fast as possible |
| `--phase-ms N` | Duration of each generated phase in milliseconds |
| `--target-ops-per-sec N` | Approximate throttle target for generated operations |
| `--phase-repeat N` | Repeat the template multiple times to observe selector stability |

Realtime mode still uses the same `StrategyDescriptor` path as benchmark mode,
so fixed allocators and `adaptive` remain comparable. It is intended for
observation, not for throughput measurement.

## JSON Output

All strategies include a `generated` object:

```json
{
  "strategy": "ptmalloc",
  "benchmark": "generated_workload",
  "ops_per_sec": 320000,
  "generated": {
    "template": "adaptive_mix",
    "allocs": 9771,
    "frees": 9771,
    "reallocs": 5000,
    "remote_frees": 3332,
    "requested_bytes": 527229532,
    "peak_live_bytes": 26162512
  }
}
```

Adaptive runs additionally include the normal `adaptive` JSON object with mode,
mode-switch, storage, and feature telemetry.

## Fair Comparison

The generated workload is deterministic for a fixed `--seed`, `--iters`,
`--slots`, `--threads`, and template. This makes it suitable for comparing the
adaptive backend against fixed allocator implementations:

```bash
for s in hybrid ptmalloc tcmalloc_like jemalloc_like mimalloc_like adaptive libc; do
  ./build/bench_runner --strategy "$s" --bench generated_workload \
    --workload-template adaptive_mix --iters 50000 --slots 2048 --threads 4 --json
done
```

Interpret adaptive-specific mode switching alongside common metrics such as
ops/sec, peak RSS, requested bytes, peak live bytes, remote frees, and phase
shape.
