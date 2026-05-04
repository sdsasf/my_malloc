#pragma once
// AllocObserver: pluggable observer for allocation events
// NullObserver has zero overhead in release builds

#include <cstddef>

namespace my_ptmalloc {

class AllocObserver {
public:
    virtual ~AllocObserver() = default;
    virtual void on_alloc(size_t size, bool from_mmap) noexcept = 0;
    virtual void on_free(size_t size, bool to_mmap) noexcept = 0;
    virtual void on_trim(size_t released) noexcept = 0;
    virtual void on_consolidate(size_t merged) noexcept = 0;
};

// Zero-overhead observer for production
class NullObserver : public AllocObserver {
public:
    void on_alloc(size_t, bool) noexcept override {}
    void on_free(size_t, bool) noexcept override {}
    void on_trim(size_t) noexcept override {}
    void on_consolidate(size_t) noexcept override {}
};

// Stats collection
class StatsObserver : public AllocObserver {
    size_t alloc_count_ = 0;
    size_t free_count_ = 0;
    size_t mmap_count_ = 0;
    size_t total_alloc_bytes_ = 0;
    size_t total_free_bytes_ = 0;
    size_t trim_count_ = 0;
    size_t consolidate_count_ = 0;

public:
    void on_alloc(size_t size, bool from_mmap) noexcept override;
    void on_free(size_t size, bool to_mmap) noexcept override;
    void on_trim(size_t released) noexcept override;
    void on_consolidate(size_t merged) noexcept override;

    [[nodiscard]] size_t alloc_count() const noexcept { return alloc_count_; }
    [[nodiscard]] size_t free_count() const noexcept { return free_count_; }
    [[nodiscard]] size_t mmap_count() const noexcept { return mmap_count_; }
    [[nodiscard]] size_t total_alloc_bytes() const noexcept { return total_alloc_bytes_; }
    [[nodiscard]] size_t total_free_bytes() const noexcept { return total_free_bytes_; }
};

extern AllocObserver* g_observer;

} // namespace my_ptmalloc
