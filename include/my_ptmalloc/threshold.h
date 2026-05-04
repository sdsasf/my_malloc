#pragma once
// ThresholdPolicy: mmap and trim threshold management
// Static uses fixed defaults, Adaptive adjusts based on allocation patterns

#include "config.h"
#include <cstddef>

namespace my_ptmalloc {

class ThresholdPolicy {
public:
    virtual ~ThresholdPolicy() = default;
    [[nodiscard]] virtual size_t mmap_threshold() const noexcept = 0;
    [[nodiscard]] virtual size_t trim_threshold() const noexcept = 0;
    virtual void on_mmap_alloc(size_t size) noexcept = 0;
    virtual void on_mmap_free(size_t size) noexcept = 0;
};

class StaticThreshold : public ThresholdPolicy {
    size_t mmap_thresh_ = DEFAULT_MMAP_THRESHOLD;
    size_t trim_thresh_ = DEFAULT_TRIM_THRESHOLD;

public:
    [[nodiscard]] size_t mmap_threshold() const noexcept override { return mmap_thresh_; }
    [[nodiscard]] size_t trim_threshold() const noexcept override { return trim_thresh_; }
    void on_mmap_alloc(size_t) noexcept override {}
    void on_mmap_free(size_t) noexcept override {}

    void set_mmap_threshold(size_t t) noexcept { mmap_thresh_ = t; }
    void set_trim_threshold(size_t t) noexcept { trim_thresh_ = t; }
};

class AdaptiveThreshold : public ThresholdPolicy {
    size_t mmap_thresh_ = DEFAULT_MMAP_THRESHOLD;
    size_t trim_thresh_ = DEFAULT_TRIM_THRESHOLD;

public:
    [[nodiscard]] size_t mmap_threshold() const noexcept override { return mmap_thresh_; }
    [[nodiscard]] size_t trim_threshold() const noexcept override { return trim_thresh_; }
    void on_mmap_alloc(size_t size) noexcept override;
    void on_mmap_free(size_t size) noexcept override;
};

extern ThresholdPolicy* g_threshold_policy;

} // namespace my_ptmalloc
