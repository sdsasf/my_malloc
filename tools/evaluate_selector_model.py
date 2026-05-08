#!/usr/bin/env python3
"""Evaluate adaptive selector backends against fixed-mode baselines.

This script runs the same generated workloads under:
  - each fixed AdaptiveMode
  - model selector

It writes machine-readable JSON plus a Markdown report that can be copied into
docs. No third-party Python packages are required.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import time
from dataclasses import dataclass
from typing import Dict, Iterable, List, Optional, Sequence


FIXED_MODES = [
    "balanced",
    "throughput_cache",
    "deterministic_latency",
    "compact_rss",
    "fragmentation_stable",
    "cross_thread",
    "large_object",
    "hardened_debug",
]

SELECTORS = ["model"]

DEFAULT_WORKLOADS = [
    "adaptive_mix",
    "throughput_churn",
    "remote_queue",
    "large_burst",
    "rss_peak_release",
    "fragmentation_drift",
    "latency_loop",
]

DEFAULT_OBJECTIVE_METRICS = [
    "ms",
    "peak_rss_kb",
    "mapped_live_ratio",
    "fragmentation_estimate",
    "slow_path_ratio",
    "mode_switches",
    "validation_errors",
]


@dataclass
class Case:
    name: str
    fixed_mode: Optional[str] = None
    selector: Optional[str] = None


def case_name(case: Case) -> str:
    if case.fixed_mode:
        return f"fixed:{case.fixed_mode}"
    return f"selector:{case.selector}"


def run_one(args: argparse.Namespace, workload: str, seed: int, case: Case) -> Dict[str, object]:
    env = os.environ.copy()
    env["MY_MALLOC_MODE"] = "adaptive"
    env["MY_MALLOC_ADAPTIVE_MODE_WINDOW"] = str(args.mode_window)
    env["MY_MALLOC_ADAPTIVE_MODE_COOLDOWN"] = str(args.mode_cooldown)
    if case.fixed_mode:
        env["MY_MALLOC_ADAPTIVE_MODE"] = case.fixed_mode
        env["MY_MALLOC_ADAPTIVE_MODE_SELECTOR"] = "fixed"
        if case.fixed_mode == "hardened_debug":
            env["MY_MALLOC_ADAPTIVE_DEBUG_MODE"] = "1"
        else:
            env.pop("MY_MALLOC_ADAPTIVE_DEBUG_MODE", None)
    else:
        env["MY_MALLOC_ADAPTIVE_MODE"] = "auto"
        env["MY_MALLOC_ADAPTIVE_MODE_SELECTOR"] = case.selector or "rule"
        env.pop("MY_MALLOC_ADAPTIVE_DEBUG_MODE", None)

    cmd = [
        args.bench_runner,
        "--strategy", "adaptive",
        "--bench", "generated_workload",
        "--workload-template", workload,
        "--iters", str(args.iters),
        "--slots", str(args.slots),
        "--threads", str(args.threads),
        "--seed", str(seed),
        "--json",
    ]
    proc = subprocess.run(cmd, env=env, text=True, capture_output=True, check=False)
    if proc.returncode != 0:
        raise RuntimeError(
            f"{case_name(case)} on {workload} failed with exit {proc.returncode}: {proc.stderr.strip()}"
        )
    line = proc.stdout.strip().splitlines()[-1]
    result = json.loads(line)
    adaptive = result.get("adaptive", {})
    generated = result.get("generated", {})
    pool_hits = adaptive.get("pool_hits", [0, 0, 0])
    pool_misses = adaptive.get("pool_misses", [0, 0, 0])
    hit_count = sum(float(x) for x in pool_hits)
    miss_count = sum(float(x) for x in pool_misses)
    return {
        "workload": f"{workload}:seed{seed}",
        "template": workload,
        "seed": seed,
        "case": case_name(case),
        "ms": float(result.get("ms", 0.0)),
        "ops_per_sec": float(result.get("ops_per_sec", 0.0)),
        "peak_rss_kb": float(result.get("peak_rss_kb", 0.0)),
        "current_mode": adaptive.get("current_mode", ""),
        "previous_mode": adaptive.get("previous_mode", ""),
        "mode_switches": float(adaptive.get("mode_switches", 0.0)),
        "mapped_live_ratio": float(adaptive.get("mapped_live_ratio", 0.0)),
        "fragmentation_estimate": float(adaptive.get("fragmentation_estimate", 0.0)),
        "slow_path_ratio": float(adaptive.get("slow_path_ratio", 0.0)),
        "large_bytes_ratio": float(adaptive.get("large_bytes_ratio", 0.0)),
        "remote_free_ratio": float(adaptive.get("remote_free_ratio", 0.0)),
        "size_entropy": float(adaptive.get("size_entropy", 0.0)),
        "cache_hit_rate": hit_count / (hit_count + miss_count) if (hit_count + miss_count) > 0.0 else 0.0,
        "alloc_calls": int(generated.get("allocs", 0)),
        "live_bytes": int(adaptive.get("live_bytes", 0)),
        "validation_errors": int(generated.get("validation_errors", 0)),
        "raw": result,
    }


def normalize(value: float, low: float, high: float) -> float:
    if high <= low:
        return 0.0
    v = (value - low) / (high - low)
    return max(0.0, min(1.0, v))


def load_objective_weights(path: str) -> Dict[str, float]:
    try:
        with open(path, "r", encoding="utf-8") as src:
            payload = json.load(src)
        weights = payload.get("training", {}).get("objective_model", {}).get("weights", {})
        parsed = {str(k): float(v) for k, v in weights.items()}
        if parsed:
            return parsed
    except OSError:
        pass
    uniform = 1.0 / len(DEFAULT_OBJECTIVE_METRICS)
    return {metric: uniform for metric in DEFAULT_OBJECTIVE_METRICS}


def score_rows(rows: List[Dict[str, object]], weights: Dict[str, float]) -> None:
    by_workload: Dict[str, List[Dict[str, object]]] = {}
    for row in rows:
        by_workload.setdefault(str(row["workload"]), []).append(row)
    for workload_rows in by_workload.values():
        ranges = {}
        for metric in weights:
            values = [float(r[metric]) for r in workload_rows]
            ranges[metric] = (min(values), max(values))
        for row in workload_rows:
            score = 0.0
            parts = {}
            for metric, weight in weights.items():
                lo, hi = ranges[metric]
                component = normalize(float(row[metric]), lo, hi)
                parts[metric] = component
                score += weight * component
            row["score"] = score
            row["score_components"] = parts


def summarize(rows: List[Dict[str, object]], weights: Dict[str, float]) -> Dict[str, object]:
    score_rows(rows, weights)
    cases = sorted({str(r["case"]) for r in rows})
    workloads = sorted({str(r["workload"]) for r in rows})
    by_case = {}
    for case in cases:
        case_rows = [r for r in rows if r["case"] == case]
        by_case[case] = {
            "mean_score": sum(float(r["score"]) for r in case_rows) / len(case_rows),
            "mean_ops_per_sec": sum(float(r["ops_per_sec"]) for r in case_rows) / len(case_rows),
            "mean_peak_rss_kb": sum(float(r["peak_rss_kb"]) for r in case_rows) / len(case_rows),
            "mean_switches": sum(float(r["mode_switches"]) for r in case_rows) / len(case_rows),
            "best_workload_count": 0,
        }
    per_workload = {}
    for workload in workloads:
        workload_rows = sorted(
            [r for r in rows if r["workload"] == workload],
            key=lambda r: float(r["score"]),
        )
        best = workload_rows[0]
        by_case[str(best["case"])]["best_workload_count"] += 1
        fixed_rows = [r for r in workload_rows if str(r["case"]).startswith("fixed:")]
        selector_rows = [r for r in workload_rows if str(r["case"]).startswith("selector:")]
        model_rows = [r for r in workload_rows if str(r["case"]) == "selector:model"]
        best_fixed = min(fixed_rows, key=lambda r: float(r["score"]))
        best_selector = min(selector_rows, key=lambda r: float(r["score"])) if selector_rows else None
        best_model = min(model_rows, key=lambda r: float(r["score"])) if model_rows else None
        per_workload[workload] = {
            "best_overall": best["case"],
            "best_fixed": best_fixed["case"],
            "best_selector": best_selector["case"] if best_selector else "n/a",
            "best_model_selector": best_model["case"] if best_model else "n/a",
            "rows": [
                {
                    "case": r["case"],
                    "score": r["score"],
                    "ops_per_sec": r["ops_per_sec"],
                    "peak_rss_kb": r["peak_rss_kb"],
                    "current_mode": r["current_mode"],
                    "mode_switches": r["mode_switches"],
                    "mapped_live_ratio": r["mapped_live_ratio"],
                    "fragmentation_estimate": r["fragmentation_estimate"],
                    "slow_path_ratio": r["slow_path_ratio"],
                }
                for r in workload_rows
            ],
        }
    return {
        "weights": weights,
        "cases": by_case,
        "per_workload": per_workload,
    }


def fmt(v: object, digits: int = 3) -> str:
    if isinstance(v, float):
        return f"{v:.{digits}f}"
    return str(v)


def write_markdown(path: str, payload: Dict[str, object]) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    summary = payload["summary"]
    args = payload["config"]
    generated_at = payload["generated_at"]
    with open(path, "w", encoding="utf-8") as out:
        out.write("# Adaptive Selector Model Evaluation Results\n\n")
        out.write(f"Generated at: `{generated_at}`\n\n")
        out.write("These measurements compare fixed adaptive modes against the measured-cost `model` selector on the generated workload suite. When requested, the legacy `rule` selector is included only as a baseline. Lower score is better.\n\n")
        out.write("## Reproduction Command\n\n")
        out.write("```bash\n")
        out.write(
            "python3 tools/evaluate_selector_model.py "
            f"--bench-runner {args['bench_runner']} --iters {args['iters']} "
            f"--slots {args['slots']} --threads {args['threads']} "
            f"--seeds {' '.join(str(s) for s in args['seeds'])} "
            f"--mode-window {args['mode_window']} --mode-cooldown {args['mode_cooldown']} "
            f"--json-out {args['json_out']} --markdown-out {args['markdown_out']}"
        )
        if args.get("include_rule_baseline"):
            out.write(" --include-rule-baseline")
        if args.get("fixed_only"):
            out.write(" --fixed-only")
        out.write("\n")
        out.write("```\n\n")
        out.write("## Score Definition\n\n")
        out.write("Score is normalized per workload across all compared cases. The metric weights are loaded from the trained objective model summary, so the report uses the same learned objective family as training:\n\n")
        out.write("| Metric | Weight | Direction |\n|---|---:|---|\n")
        for metric, weight in summary["weights"].items():
            out.write(f"| `{metric}` | {weight:.2f} | lower is better |\n")
        out.write("\n")
        out.write("## Overall Ranking\n\n")
        out.write("| Rank | Case | Mean score | Mean ops/sec | Mean peak RSS KB | Mean switches | Best workload count |\n")
        out.write("|---:|---|---:|---:|---:|---:|---:|\n")
        ranked = sorted(summary["cases"].items(), key=lambda kv: kv[1]["mean_score"])
        for i, (case, data) in enumerate(ranked, 1):
            out.write(
                f"| {i} | `{case}` | {data['mean_score']:.3f} | "
                f"{data['mean_ops_per_sec']:.0f} | {data['mean_peak_rss_kb']:.0f} | "
                f"{data['mean_switches']:.1f} | {data['best_workload_count']} |\n"
            )
        out.write("\n")
        out.write("## Per-Workload Winners\n\n")
        out.write("| Workload | Best overall | Best fixed mode | Model selector |\n")
        out.write("|---|---|---|---|\n")
        for workload, data in summary["per_workload"].items():
            out.write(
                f"| `{workload}` | `{data['best_overall']}` | "
                f"`{data['best_fixed']}` | `{data['best_model_selector']}` |\n"
            )
        out.write("\n")
        for workload, data in summary["per_workload"].items():
            out.write(f"## `{workload}` Details\n\n")
            out.write("| Case | Score | Ops/sec | Peak RSS KB | Final mode | Switches | mapped/live | frag | slow-path |\n")
            out.write("|---|---:|---:|---:|---|---:|---:|---:|---:|\n")
            for row in data["rows"]:
                out.write(
                    f"| `{row['case']}` | {row['score']:.3f} | {row['ops_per_sec']:.0f} | "
                    f"{row['peak_rss_kb']:.0f} | `{row['current_mode']}` | "
                    f"{row['mode_switches']:.0f} | {row['mapped_live_ratio']:.2f} | "
                    f"{row['fragmentation_estimate']:.3f} | {row['slow_path_ratio']:.3f} |\n"
                )
            out.write("\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--bench-runner", default="./build/bench_runner")
    parser.add_argument("--workloads", nargs="*", default=DEFAULT_WORKLOADS)
    parser.add_argument("--iters", type=int, default=30000)
    parser.add_argument("--slots", type=int, default=512)
    parser.add_argument("--threads", type=int, default=2)
    parser.add_argument("--seeds", nargs="*", type=int, default=[12345])
    parser.add_argument("--mode-window", type=int, default=64)
    parser.add_argument("--mode-cooldown", type=int, default=0)
    parser.add_argument("--json-out", default="models/selector_evaluation_results.json")
    parser.add_argument("--markdown-out", default="docs/adaptive_selector_model_results.md")
    parser.add_argument("--objective-summary", default="models/selector_training_summary.json")
    parser.add_argument("--include-hardened-debug", action="store_true")
    parser.add_argument("--fixed-only", action="store_true")
    parser.add_argument("--include-rule-baseline", action="store_true")
    args = parser.parse_args()

    cases = [Case(name=f"fixed:{m}", fixed_mode=m) for m in FIXED_MODES]
    if not args.include_hardened_debug:
        cases = [c for c in cases if c.fixed_mode != "hardened_debug"]
    if not args.fixed_only:
        cases += [Case(name="selector:model", selector="model")]
        if args.include_rule_baseline:
            cases += [Case(name="selector:rule", selector="rule")]

    rows: List[Dict[str, object]] = []
    for seed in args.seeds:
        for workload in args.workloads:
            for case in cases:
                print(f"running {workload} seed={seed} {case_name(case)}", flush=True)
                rows.append(run_one(args, workload, seed, case))

    objective_weights = load_objective_weights(args.objective_summary)
    payload = {
        "generated_at": time.strftime("%Y-%m-%d %H:%M:%S %z"),
        "config": {
            "bench_runner": args.bench_runner,
            "workloads": args.workloads,
            "seeds": args.seeds,
            "iters": args.iters,
            "slots": args.slots,
            "threads": args.threads,
            "mode_window": args.mode_window,
            "mode_cooldown": args.mode_cooldown,
            "json_out": args.json_out,
            "markdown_out": args.markdown_out,
            "objective_summary": args.objective_summary,
            "include_hardened_debug": args.include_hardened_debug,
            "fixed_only": args.fixed_only,
            "include_rule_baseline": args.include_rule_baseline,
        },
        "rows": rows,
        "summary": summarize(rows, objective_weights),
    }
    os.makedirs(os.path.dirname(args.json_out), exist_ok=True)
    with open(args.json_out, "w", encoding="utf-8") as out:
        json.dump(payload, out, indent=2, sort_keys=True)
        out.write("\n")
    write_markdown(args.markdown_out, payload)
    print(json.dumps(payload["summary"]["cases"], indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
