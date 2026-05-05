# Adaptive Allocator Mode

`MY_MALLOC_MODE=adaptive` selects a simple teaching policy that routes allocations by size.

## Current Policy

```text
size <= 256      -> mimalloc_like
size <= 4096     -> tcmalloc_like
size > 4096      -> direct mmap-backed large allocation
```

This is intentionally simple. It is not a reinforcement-learning allocator yet. It exists so that the project has a concrete adaptive baseline before adding dynamic policies.

## Why This Policy

| Size range | Selected path | Reason |
|---|---|---|
| Tiny objects | mimalloc-like | Tiny objects often appear in cross-thread/server workloads; owner remote-free is useful to study. |
| Small objects | tcmalloc-like | Thread cache plus central free list gives a clear size-class batching baseline. |
| Large objects | mmap | Keeps large allocations out of small-object caches. |

## Future Adaptive Signals

A stronger adaptive allocator should observe:

- allocation histogram by size class;
- remote-free ratio;
- thread cache hit rate;
- central refill/drain rate;
- arena lock contention;
- live bytes vs mapped bytes;
- empty pages/spans;
- p99 allocation latency.

## Future Policies

Practical next steps:

1. Add structured counters for each mode.
2. Add heuristic policy tables configurable from environment variables.
3. Add repeated benchmark result collection.
4. Only then experiment with learned policies.

Example heuristic:

```text
if remote_free_ratio > threshold:
    prefer mimalloc-like ownership for that size class

if central_refill_rate is high:
    increase tcmalloc-like batch size

if mapped_bytes >> live_bytes:
    release empty spans/pages more aggressively
```

Run:

```bash
./build/allocator_validate --strategy adaptive
./build/bench_runner --strategy adaptive --profile all --json
```
