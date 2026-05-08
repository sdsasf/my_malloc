#include "my_ptmalloc/adaptive_model_selector.h"

#include "my_ptmalloc/adaptive_runtime.h"
#include "my_ptmalloc/generated_selector_model.h"

#include <cmath>

namespace my_ptmalloc {

namespace {

static double safe_log1p(double v) noexcept {
    return std::log1p(v > 0.0 ? v : 0.0);
}

static void build_features(const WorkloadFeatures& f,
                           double out[generated_selector_model::FEATURE_COUNT]) noexcept {
    out[generated_selector_model::LargeBytesRatio] = f.large_bytes_ratio;
    out[generated_selector_model::RemoteFreeRatio] = f.remote_free_ratio;
    out[generated_selector_model::MappedLiveRatio] = f.mapped_live_ratio;
    out[generated_selector_model::SlowPathRatio] = f.slow_path_ratio;
    out[generated_selector_model::SizeEntropy] = f.size_entropy;
    out[generated_selector_model::InternalFragRatio] = f.internal_frag_ratio;
    out[generated_selector_model::CacheHitRate] = f.cache_hit_rate;
    out[generated_selector_model::SafetyErrorRate] = f.safety_error_rate;
    out[generated_selector_model::LogAllocCalls] = safe_log1p(static_cast<double>(f.alloc_calls));
    out[generated_selector_model::LogLiveBytes] = safe_log1p(static_cast<double>(f.live_bytes));
}

static double predict_cost(const WorkloadFeatures& f,
                           AdaptiveModeId candidate,
                           AdaptiveModeId current) noexcept {
    double features[generated_selector_model::FEATURE_COUNT]{};
    build_features(f, features);
    size_t mode = adaptive_mode_index(candidate);
    double score = generated_selector_model::BIAS[mode];
    for (const auto& tree : generated_selector_model::TREES) {
        if (tree.mode != mode) continue;
        double v = features[tree.feature];
        score += v <= tree.threshold ? tree.left_value : tree.right_value;
    }
    if (candidate != current) score += generated_selector_model::SWITCH_COST;
    return score;
}

} // namespace

AdaptiveModelDecision adaptive_model_select_mode(const WorkloadFeatures& f,
                                                 AdaptiveModeId current) noexcept {
    AdaptiveModelDecision decision{};
    decision.mode = AdaptiveModeId::Balanced;
    decision.available = true;
    double best = 1e100;
    double second = 1e100;
    for (size_t i = 0; i < ADAPTIVE_MODE_COUNT; ++i) {
        AdaptiveModeId candidate = static_cast<AdaptiveModeId>(i);
        double cost = predict_cost(f, candidate, current);
        decision.cost[i] = cost;
        if (cost < best) {
            second = best;
            best = cost;
            decision.mode = candidate;
        } else if (cost < second) {
            second = cost;
        }
    }
    decision.confidence = second < 1e90 ? second - best : 0.0;
    return decision;
}

const char* adaptive_model_id() noexcept {
    return generated_selector_model::MODEL_ID;
}

} // namespace my_ptmalloc
