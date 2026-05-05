#pragma once
// Experimental allocator-lab controls, statistics, and tracing.

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace my_ptmalloc {

enum class AllocMode : uint8_t {
    Hybrid,
    PtmallocOnly,
};

enum class AllocOp : uint8_t {
    Malloc,
    Free,
    Realloc,
    Calloc,
};

enum class AllocPath : uint8_t {
    None,
    Slab,
    Tcache,
    Arena,
    Mmap,
};

struct AllocStatsSnapshot {
    uint64_t malloc_calls;
    uint64_t free_calls;
    uint64_t realloc_calls;
    uint64_t calloc_calls;

    uint64_t slab_allocs;
    uint64_t slab_frees;
    uint64_t slab_refills;
    uint64_t slab_drains;
    uint64_t slab_new_slabs;

    uint64_t tcache_allocs;
    uint64_t tcache_frees;
    uint64_t arena_allocs;
    uint64_t arena_frees;
    uint64_t mmap_frees;
};

struct AllocTraceEvent {
    uint64_t seq;
    uintptr_t ptr;
    size_t size;
    AllocOp op;
    AllocPath path;
};

void allocator_lab_init() noexcept;
[[nodiscard]] AllocMode allocator_mode() noexcept;
[[nodiscard]] bool allocator_stats_enabled() noexcept;
[[nodiscard]] bool allocator_trace_enabled() noexcept;

extern bool g_allocator_stats_enabled;
extern bool g_allocator_trace_enabled;

[[nodiscard]] inline bool allocator_stats_enabled_fast() noexcept {
    return g_allocator_stats_enabled;
}

[[nodiscard]] inline bool allocator_trace_enabled_fast() noexcept {
    return g_allocator_trace_enabled;
}

void stats_record_alloc(AllocPath path) noexcept;
void stats_record_free(AllocPath path) noexcept;
void stats_record_realloc() noexcept;
void stats_record_calloc() noexcept;
void stats_record_slab_refill() noexcept;
void stats_record_slab_drain() noexcept;
void stats_record_slab_new() noexcept;

void trace_record(AllocOp op, AllocPath path, size_t size, void* ptr) noexcept;

[[nodiscard]] AllocStatsSnapshot my_malloc_stats_snapshot() noexcept;
void my_malloc_stats_reset() noexcept;
void my_malloc_dump_stats_json(FILE* out) noexcept;

} // namespace my_ptmalloc
