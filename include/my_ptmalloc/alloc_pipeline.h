#pragma once
// AllocPipeline: chain-of-responsibility allocation strategy
// Each strategy is tried in priority order until one succeeds

#include "arena.h"
#include <vector>

namespace my_ptmalloc {

class AllocStrategy {
public:
    virtual ~AllocStrategy() = default;
    // Try to allocate nb bytes. Returns true if successful, result set to user ptr.
    virtual bool try_alloc(Arena& arena, size_t nb, void*& result) noexcept = 0;
};

// Strategy implementations
class TcacheAlloc : public AllocStrategy {
public:
    bool try_alloc(Arena& arena, size_t nb, void*& result) noexcept override;
};

class FastbinAlloc : public AllocStrategy {
public:
    bool try_alloc(Arena& arena, size_t nb, void*& result) noexcept override;
};

class SmallbinAlloc : public AllocStrategy {
public:
    bool try_alloc(Arena& arena, size_t nb, void*& result) noexcept override;
};

class UnsortedAlloc : public AllocStrategy {
public:
    bool try_alloc(Arena& arena, size_t nb, void*& result) noexcept override;
};

class LargebinAlloc : public AllocStrategy {
public:
    bool try_alloc(Arena& arena, size_t nb, void*& result) noexcept override;
};

class TopChunkAlloc : public AllocStrategy {
public:
    bool try_alloc(Arena& arena, size_t nb, void*& result) noexcept override;
};

class SysAlloc : public AllocStrategy {
public:
    bool try_alloc(Arena& arena, size_t nb, void*& result) noexcept override;
};

// Pipeline: chains all strategies
class AllocPipeline {
    TcacheAlloc   tcache_;
    FastbinAlloc  fastbin_;
    SmallbinAlloc smallbin_;
    UnsortedAlloc unsorted_;
    LargebinAlloc largebin_;
    TopChunkAlloc topchunk_;
    SysAlloc      sys_;

    std::vector<AllocStrategy*> strategies_;

public:
    AllocPipeline() noexcept;

    // Execute pipeline: try each strategy in order
    bool execute(Arena& arena, size_t nb, void*& result) noexcept;
};

extern AllocPipeline* g_alloc_pipeline;

} // namespace my_ptmalloc
