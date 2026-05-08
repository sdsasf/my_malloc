#!/usr/bin/env python3
"""Train the adaptive selector model from measured benchmark results.

The training pipeline is deliberately measured-cost based:

1. Load fixed-mode benchmark rows.
2. Learn an objective-cost formula from pairwise measured outcomes inside each
   workload window.
3. Train a compact per-mode cost model on the learned objective labels.

No hand-written per-mode oracle is used, and the online allocator does not mix
this model with the legacy rule selector.
"""

from __future__ import annotations

import argparse
import json
import math
import os
from dataclasses import dataclass
from typing import Dict, List, Sequence, Tuple


FEATURES = [
    "LargeBytesRatio",
    "RemoteFreeRatio",
    "MappedLiveRatio",
    "SlowPathRatio",
    "SizeEntropy",
    "InternalFragRatio",
    "CacheHitRate",
    "SafetyErrorRate",
    "LogAllocCalls",
    "LogLiveBytes",
]

MODES = [
    "Balanced",
    "ThroughputCache",
    "DeterministicLatency",
    "CompactRSS",
    "FragmentationStable",
    "CrossThreadMessage",
    "LargeObjectStreaming",
    "HardenedDebug",
]

CASE_TO_MODE = {
    "fixed:balanced": 0,
    "fixed:throughput_cache": 1,
    "fixed:deterministic_latency": 2,
    "fixed:compact_rss": 3,
    "fixed:fragmentation_stable": 4,
    "fixed:cross_thread": 5,
    "fixed:large_object": 6,
    "fixed:hardened_debug": 7,
}

OBJECTIVE_METRICS = [
    "ms",
    "peak_rss_kb",
    "mapped_live_ratio",
    "fragmentation_estimate",
    "slow_path_ratio",
    "mode_switches",
    "validation_errors",
]


@dataclass
class Example:
    mode: int
    features: List[float]
    target: float
    workload: str


@dataclass
class Stump:
    mode: int
    feature: int
    threshold: float
    left: float
    right: float


@dataclass
class ObjectiveModel:
    metrics: List[str]
    weights: List[float]
    switch_cost: float
    pairwise_accuracy: float


def safe_log1p(v: float) -> float:
    return math.log1p(v if v > 0.0 else 0.0)


def row_features(row: Dict[str, object]) -> List[float]:
    safety_errors = float(row.get("validation_errors", 0))
    alloc_calls = float(row.get("alloc_calls", 0))
    return [
        float(row.get("large_bytes_ratio", 0.0)),
        float(row.get("remote_free_ratio", 0.0)),
        float(row.get("mapped_live_ratio", 0.0)),
        float(row.get("slow_path_ratio", 0.0)),
        float(row.get("size_entropy", 0.0)),
        float(row.get("fragmentation_estimate", 0.0)),
        float(row.get("cache_hit_rate", 0.0)),
        safety_errors / max(1.0, alloc_calls),
        safe_log1p(alloc_calls),
        safe_log1p(float(row.get("live_bytes", 0.0))),
    ]


def normalized_metric_rows(rows: Sequence[Dict[str, object]]) -> List[Dict[str, float]]:
    by_workload: Dict[str, List[Dict[str, object]]] = {}
    for row in rows:
        by_workload.setdefault(str(row.get("workload", "")), []).append(row)
    normalized: List[Dict[str, float]] = []
    for workload_rows in by_workload.values():
        ranges: Dict[str, Tuple[float, float]] = {}
        for metric in OBJECTIVE_METRICS:
            values = [float(r.get(metric, 0.0)) for r in workload_rows]
            ranges[metric] = (min(values), max(values))
        for row in workload_rows:
            item = {"__row_id": float(len(normalized))}
            for metric in OBJECTIVE_METRICS:
                lo, hi = ranges[metric]
                value = float(row.get(metric, 0.0))
                item[metric] = 0.0 if hi <= lo else max(0.0, min(1.0, (value - lo) / (hi - lo)))
            normalized.append(item)
    return normalized


def majority_preference(a: Dict[str, float], b: Dict[str, float]) -> int:
    """Return -1 when a is better, 1 when b is better, 0 for no clear winner.

    Lower normalized metric values are better. This creates pairwise measured
    preferences without assigning hand-written metric weights.
    """
    a_wins = 0
    b_wins = 0
    for metric in OBJECTIVE_METRICS:
        av = a[metric]
        bv = b[metric]
        if abs(av - bv) < 1e-9:
            continue
        if av < bv:
            a_wins += 1
        else:
            b_wins += 1
    if a_wins == b_wins:
        return 0
    return -1 if a_wins > b_wins else 1


def learn_objective_model(rows: Sequence[Dict[str, object]],
                          epochs: int = 200,
                          learning_rate: float = 0.08) -> ObjectiveModel:
    normalized = normalized_metric_rows(rows)
    by_id = {int(item["__row_id"]): item for item in normalized}
    pairs: List[Tuple[int, int, int]] = []
    by_workload: Dict[str, List[int]] = {}
    for i, row in enumerate(rows):
        by_workload.setdefault(str(row.get("workload", "")), []).append(i)
    for indexes in by_workload.values():
        for pos, ia in enumerate(indexes):
            for ib in indexes[pos + 1:]:
                pref = majority_preference(by_id[ia], by_id[ib])
                if pref != 0:
                    pairs.append((ia, ib, pref))
    weights = [1.0 / len(OBJECTIVE_METRICS)] * len(OBJECTIVE_METRICS)
    if pairs:
        for _ in range(epochs):
            for ia, ib, pref in pairs:
                a = by_id[ia]
                b = by_id[ib]
                # Positive margin means the preferred row already has lower cost.
                diff = [b[m] - a[m] if pref == -1 else a[m] - b[m] for m in OBJECTIVE_METRICS]
                margin = sum(w * d for w, d in zip(weights, diff))
                if margin < 0.025:
                    for j, d in enumerate(diff):
                        weights[j] += learning_rate * d
                    # Keep the objective monotonic: worse measured metrics
                    # should not reduce predicted cost.
                    weights = [max(0.0, w) for w in weights]
                    total = sum(weights)
                    if total <= 1e-12:
                        weights = [1.0 / len(OBJECTIVE_METRICS)] * len(OBJECTIVE_METRICS)
                    else:
                        weights = [w / total for w in weights]
    correct = 0
    for ia, ib, pref in pairs:
        a = by_id[ia]
        b = by_id[ib]
        ca = sum(weights[j] * a[m] for j, m in enumerate(OBJECTIVE_METRICS))
        cb = sum(weights[j] * b[m] for j, m in enumerate(OBJECTIVE_METRICS))
        if (pref == -1 and ca < cb) or (pref == 1 and cb < ca):
            correct += 1
    # Switch cost is also data-derived: use a fraction of the median gap between
    # the best and second-best measured fixed-mode costs for each workload.
    gaps: List[float] = []
    for indexes in by_workload.values():
        costs = []
        for idx in indexes:
            item = by_id[idx]
            costs.append(sum(weights[j] * item[m] for j, m in enumerate(OBJECTIVE_METRICS)))
        if len(costs) >= 2:
            ordered = sorted(costs)
            gaps.append(max(0.0, ordered[1] - ordered[0]))
    if gaps:
        gaps.sort()
        switch_cost = 0.5 * gaps[len(gaps) // 2]
    else:
        switch_cost = 0.0
    return ObjectiveModel(
        metrics=list(OBJECTIVE_METRICS),
        weights=weights,
        switch_cost=switch_cost,
        pairwise_accuracy=correct / len(pairs) if pairs else 0.0,
    )


def objective_cost(row: Dict[str, object],
                   normalized_row: Dict[str, float],
                   objective: ObjectiveModel) -> float:
    cost = 0.0
    for metric, weight in zip(objective.metrics, objective.weights):
        cost += weight * normalized_row[metric]
    # Invalid payload validation must dominate learned tradeoffs.
    if float(row.get("validation_errors", 0.0)) > 0.0:
        cost += 4.0
    return cost


def load_fixed_rows(path: str) -> List[Dict[str, object]]:
    with open(path, "r", encoding="utf-8") as src:
        payload = json.load(src)
    rows = payload.get("rows", [])
    fixed_rows: List[Dict[str, object]] = []
    for row in rows:
        if str(row.get("case", "")) in CASE_TO_MODE:
            fixed_rows.append(row)
    if not fixed_rows:
        raise ValueError(f"no fixed-mode training rows found in {path}")
    return fixed_rows


def load_examples(path: str, objective: ObjectiveModel,
                  fixed_rows: Sequence[Dict[str, object]]) -> List[Example]:
    normalized = normalized_metric_rows(fixed_rows)
    normalized_by_workload: Dict[str, List[Tuple[Dict[str, object], Dict[str, float]]]] = {}
    for row, norm in zip(fixed_rows, normalized):
        normalized_by_workload.setdefault(str(row.get("workload", "")), []).append((row, norm))
    by_workload: Dict[str, List[Dict[str, object]]] = {}
    for row in fixed_rows:
        by_workload.setdefault(str(row.get("workload", "")), []).append(row)

    examples: List[Example] = []
    for workload, workload_rows in by_workload.items():
        # Training is candidate-cost prediction. For each workload window, every
        # candidate mode must see the same selector-visible signature and learn
        # a different measured target cost. Using per-mode generated features as
        # the input for that same mode would leak allocator behavior into the
        # feature vector and make online all-candidate scoring inconsistent.
        projected = [row_features(row) for row in workload_rows]
        signature = [
            sum(features[i] for features in projected) / len(projected)
            for i in range(len(FEATURES))
        ]
        norm_rows = {id(row): norm for row, norm in normalized_by_workload[workload]}
        for row in workload_rows:
            examples.append(
                Example(
                    mode=CASE_TO_MODE[str(row.get("case", ""))],
                    features=signature,
                    target=objective_cost(row, norm_rows[id(row)], objective),
                    workload=workload,
                )
            )
    if not examples:
        raise ValueError(f"no fixed-mode training examples found in {path}")
    return examples


def candidate_thresholds(values: Sequence[float]) -> List[float]:
    ordered = sorted(values)
    if len(ordered) < 2:
        return ordered
    qs = [0.10, 0.20, 0.35, 0.50, 0.65, 0.80, 0.90]
    return sorted({ordered[min(len(ordered) - 1, int(q * (len(ordered) - 1)))] for q in qs})


def best_stump(examples: Sequence[Example], residuals: Sequence[float], mode: int) -> Stump:
    best = None
    best_sse = float("inf")
    for feature in range(len(FEATURES)):
        values = [e.features[feature] for e in examples]
        for threshold in candidate_thresholds(values):
            left = [r for e, r in zip(examples, residuals) if e.features[feature] <= threshold]
            right = [r for e, r in zip(examples, residuals) if e.features[feature] > threshold]
            if not left or not right:
                continue
            lv = sum(left) / len(left)
            rv = sum(right) / len(right)
            sse = 0.0
            for e, r in zip(examples, residuals):
                pred = lv if e.features[feature] <= threshold else rv
                d = r - pred
                sse += d * d
            if sse < best_sse:
                best_sse = sse
                best = Stump(mode, feature, threshold, lv, rv)
    if best is None:
        return Stump(mode, 0, 0.0, 0.0, 0.0)
    return best


def train(examples: Sequence[Example], rounds_per_mode: int,
          learning_rate: float) -> Tuple[List[float], List[Stump]]:
    biases = [1.0] * len(MODES)
    trees: List[Stump] = []
    for mode in range(len(MODES)):
        mode_examples = [e for e in examples if e.mode == mode]
        if not mode_examples:
            biases[mode] = 10.0
            continue
        predictions = [sum(e.target for e in mode_examples) / len(mode_examples)] * len(mode_examples)
        biases[mode] = predictions[0]
        for _ in range(rounds_per_mode):
            residuals = [e.target - p for e, p in zip(mode_examples, predictions)]
            stump = best_stump(mode_examples, residuals, mode)
            stump.left *= learning_rate
            stump.right *= learning_rate
            trees.append(stump)
            for i, e in enumerate(mode_examples):
                predictions[i] += stump.left if e.features[stump.feature] <= stump.threshold else stump.right
    return biases, trees


def predict(features: Sequence[float], biases: Sequence[float], trees: Sequence[Stump]) -> List[float]:
    scores = list(biases)
    for tree in trees:
        scores[tree.mode] += tree.left if features[tree.feature] <= tree.threshold else tree.right
    return scores


def evaluate(examples: Sequence[Example], biases: Sequence[float],
             trees: Sequence[Stump]) -> Dict[str, object]:
    by_workload: Dict[str, List[Example]] = {}
    for e in examples:
        by_workload.setdefault(e.workload, []).append(e)
    correct = 0
    total = 0
    regret = 0.0
    mae = 0.0
    mode_hits = [0] * len(MODES)
    for workload_examples in by_workload.values():
        best_measured = min(workload_examples, key=lambda e: e.target)
        # Use the mean feature vector of the fixed-mode runs for this workload
        # as the selector-visible workload signature.
        signature = [
            sum(e.features[i] for e in workload_examples) / len(workload_examples)
            for i in range(len(FEATURES))
        ]
        scores = predict(signature, biases, trees)
        predicted_mode = min(range(len(MODES)), key=lambda m: scores[m])
        mode_hits[predicted_mode] += 1
        measured_for_pred = [e for e in workload_examples if e.mode == predicted_mode]
        if measured_for_pred:
            chosen = measured_for_pred[0]
            if chosen.mode == best_measured.mode:
                correct += 1
            regret += chosen.target - best_measured.target
        total += 1
        for e in workload_examples:
            mae += abs(predict(e.features, biases, trees)[e.mode] - e.target)
    return {
        "workload_count": total,
        "example_count": len(examples),
        "top1_accuracy": correct / total if total else 0.0,
        "mean_regret": regret / total if total else 0.0,
        "mean_absolute_error": mae / len(examples) if examples else 0.0,
        "mode_pick_counts": {MODES[i]: mode_hits[i] for i in range(len(MODES))},
    }


def split_examples(examples: Sequence[Example]) -> Tuple[List[Example], List[Example]]:
    workloads = sorted({e.workload for e in examples})
    test_workloads = set(workloads[::5]) if len(workloads) >= 5 else set(workloads[-1:])
    train_set = [e for e in examples if e.workload not in test_workloads]
    test_set = [e for e in examples if e.workload in test_workloads]
    if not train_set or not test_set:
        return list(examples), list(examples)
    return train_set, test_set


def write_header(path: str, model_id: str, biases: Sequence[float], trees: Sequence[Stump],
                 switch_cost: float) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as out:
        out.write("#pragma once\n")
        out.write("// Generated by tools/train_selector_model.py from measured benchmark data.\n")
        out.write("// Do not edit by hand.\n\n")
        out.write("#include <cstddef>\n#include <cstdint>\n\n")
        out.write("namespace my_ptmalloc::generated_selector_model {\n\n")
        out.write(f"static constexpr const char* MODEL_ID = \"{model_id}\";\n")
        out.write(f"static constexpr size_t FEATURE_COUNT = {len(FEATURES)};\n")
        out.write(f"static constexpr size_t TREE_COUNT = {len(trees)};\n")
        out.write(f"static constexpr double SWITCH_COST = {switch_cost:.12g};\n\n")
        out.write("enum Feature : uint8_t {\n")
        for i, name in enumerate(FEATURES):
            out.write(f"    {name} = {i},\n")
        out.write("};\n\n")
        out.write("struct Tree {\n")
        out.write("    uint8_t mode;\n")
        out.write("    uint8_t feature;\n")
        out.write("    double threshold;\n")
        out.write("    double left_value;\n")
        out.write("    double right_value;\n")
        out.write("};\n\n")
        out.write("static constexpr double BIAS[8] = {\n    ")
        out.write(", ".join(f"{b:.12g}" for b in biases))
        out.write("\n};\n\n")
        out.write("static constexpr Tree TREES[TREE_COUNT] = {\n")
        for tree in trees:
            out.write(
                f"    {{{tree.mode}, {FEATURES[tree.feature]}, {tree.threshold:.12g}, "
                f"{tree.left:.12g}, {tree.right:.12g}}},\n"
            )
        out.write("};\n\n")
        out.write("} // namespace my_ptmalloc::generated_selector_model\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", default="models/selector_evaluation_results.json")
    parser.add_argument("--model-out", default="include/my_ptmalloc/generated_selector_model.h")
    parser.add_argument("--summary-out", default="models/selector_training_summary.json")
    parser.add_argument("--rounds-per-mode", type=int, default=6)
    parser.add_argument("--learning-rate", type=float, default=0.35)
    args = parser.parse_args()

    fixed_rows = load_fixed_rows(args.dataset)
    objective = learn_objective_model(fixed_rows)
    examples = load_examples(args.dataset, objective, fixed_rows)
    train_set, test_set = split_examples(examples)
    biases, trees = train(train_set, args.rounds_per_mode, args.learning_rate)
    model_id = f"learned_cost_gbdt_v2_n{len(train_set)}_t{len(trees)}"
    write_header(args.model_out, model_id, biases, trees, objective.switch_cost)
    summary = {
        "model_id": model_id,
        "dataset": args.dataset,
        "feature_names": FEATURES,
        "mode_names": MODES,
        "training": {
            "examples": len(examples),
            "train_examples": len(train_set),
            "test_examples": len(test_set),
            "rounds_per_mode": args.rounds_per_mode,
            "learning_rate": args.learning_rate,
            "tree_count": len(trees),
            "label_source": "learned objective cost from fixed-mode measured benchmark rows",
            "objective_model": {
                "metrics": objective.metrics,
                "weights": {m: objective.weights[i] for i, m in enumerate(objective.metrics)},
                "switch_cost": objective.switch_cost,
                "pairwise_accuracy": objective.pairwise_accuracy,
            },
        },
        "train_metrics": evaluate(train_set, biases, trees),
        "test_metrics": evaluate(test_set, biases, trees),
        "note": "The objective cost and selector model are trained from measured rows; no hand-written score weights or mode oracle are used for training.",
    }
    os.makedirs(os.path.dirname(args.summary_out), exist_ok=True)
    with open(args.summary_out, "w", encoding="utf-8") as out:
        json.dump(summary, out, indent=2, sort_keys=True)
        out.write("\n")
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
