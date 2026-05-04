#pragma once
// CoalescePolicy: configurable chunk merging strategy
// glibc default is eager (always merge), but adaptive can be better for some workloads

#include "types.h"
#include <cstdint>

namespace my_ptmalloc {

class CoalescePolicy {
public:
    virtual ~CoalescePolicy() = default;
    virtual bool should_merge(ChunkSize cs) noexcept = 0;
};

// Always merge (glibc default behavior)
class EagerCoalesce : public CoalescePolicy {
public:
    bool should_merge(ChunkSize) noexcept override { return true; }
};

// Adaptive: skip merging small chunks under high churn
class AdaptiveCoalesce : public CoalescePolicy {
    uint64_t merge_count_ = 0;
    uint64_t split_count_ = 0;

public:
    bool should_merge(ChunkSize cs) noexcept override;
    void on_merge() noexcept { merge_count_++; }
    void on_split() noexcept { split_count_++; }
};

// Global coalesce policy
extern CoalescePolicy* g_coalesce_policy;

} // namespace my_ptmalloc
