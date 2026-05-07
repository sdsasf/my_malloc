// Allocator lab controls, statistics, and optional tracing.

#include "my_ptmalloc/allocator_lab.h"

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

namespace my_ptmalloc {

namespace {

constexpr size_t TRACE_RING_SIZE = 4096;

struct AllocStatsAtomic {
    std::atomic<uint64_t> malloc_calls{0};
    std::atomic<uint64_t> free_calls{0};
    std::atomic<uint64_t> realloc_calls{0};
    std::atomic<uint64_t> calloc_calls{0};

    std::atomic<uint64_t> slab_allocs{0};
    std::atomic<uint64_t> slab_frees{0};
    std::atomic<uint64_t> slab_refills{0};
    std::atomic<uint64_t> slab_drains{0};
    std::atomic<uint64_t> slab_new_slabs{0};

    std::atomic<uint64_t> tcache_allocs{0};
    std::atomic<uint64_t> tcache_frees{0};
    std::atomic<uint64_t> arena_allocs{0};
    std::atomic<uint64_t> arena_frees{0};
    std::atomic<uint64_t> mmap_frees{0};
};

AllocMode g_mode = AllocMode::Hybrid;
bool g_mode_forced = false;
pthread_once_t g_lab_once = PTHREAD_ONCE_INIT;
AllocStatsAtomic g_stats;
std::atomic<uint64_t> g_trace_seq{0};
AllocTraceEvent g_trace_ring[TRACE_RING_SIZE]{};

[[nodiscard]] bool env_equals(const char* value, const char* expected) noexcept {
    return value && std::strcmp(value, expected) == 0;
}

} // namespace

bool g_allocator_stats_enabled = false;
bool g_allocator_trace_enabled = false;

void allocator_lab_init_once() noexcept {
    const char* mode = std::getenv("MY_MALLOC_MODE");
    if (g_mode_forced) {
        // Strategy tools can force mode without calling setenv/putenv, because
        // environment mutation can allocate and recursively initialize malloc.
    } else if (env_equals(mode, "ptmalloc")) {
        g_mode = AllocMode::PtmallocOnly;
    } else if (env_equals(mode, "tcmalloc") || env_equals(mode, "tcmalloc_like")) {
        g_mode = AllocMode::TcmallocLike;
    } else if (env_equals(mode, "jemalloc") || env_equals(mode, "jemalloc_like")) {
        g_mode = AllocMode::JemallocLike;
    } else if (env_equals(mode, "mimalloc") || env_equals(mode, "mimalloc_like")) {
        g_mode = AllocMode::MimallocLike;
    } else if (env_equals(mode, "adaptive")) {
        g_mode = AllocMode::Adaptive;
    } else {
        g_mode = AllocMode::Hybrid;
    }

    const char* trace = std::getenv("MY_MALLOC_TRACE");
    g_allocator_trace_enabled = env_equals(trace, "1") || env_equals(trace, "true");

    const char* stats = std::getenv("MY_MALLOC_STATS");
    g_allocator_stats_enabled = g_allocator_trace_enabled ||
                                env_equals(stats, "1") || env_equals(stats, "true");
}

void allocator_lab_init() noexcept {
    pthread_once(&g_lab_once, allocator_lab_init_once);
}

void allocator_lab_force_mode(AllocMode mode) noexcept {
    g_mode = mode;
    g_mode_forced = true;
}

AllocMode allocator_mode() noexcept {
    allocator_lab_init();
    return g_mode;
}

bool allocator_trace_enabled() noexcept {
    allocator_lab_init();
    return g_allocator_trace_enabled;
}

bool allocator_stats_enabled() noexcept {
    allocator_lab_init();
    return g_allocator_stats_enabled;
}

void stats_record_alloc(AllocPath path) noexcept {
    g_stats.malloc_calls.fetch_add(1, std::memory_order_relaxed);
    switch (path) {
        case AllocPath::Slab:
            g_stats.slab_allocs.fetch_add(1, std::memory_order_relaxed);
            break;
        case AllocPath::Tcache:
            g_stats.tcache_allocs.fetch_add(1, std::memory_order_relaxed);
            break;
        case AllocPath::Arena:
            g_stats.arena_allocs.fetch_add(1, std::memory_order_relaxed);
            break;
        default:
            break;
    }
}

void stats_record_free(AllocPath path) noexcept {
    g_stats.free_calls.fetch_add(1, std::memory_order_relaxed);
    switch (path) {
        case AllocPath::Slab:
            g_stats.slab_frees.fetch_add(1, std::memory_order_relaxed);
            break;
        case AllocPath::Tcache:
            g_stats.tcache_frees.fetch_add(1, std::memory_order_relaxed);
            break;
        case AllocPath::Arena:
            g_stats.arena_frees.fetch_add(1, std::memory_order_relaxed);
            break;
        case AllocPath::Mmap:
            g_stats.mmap_frees.fetch_add(1, std::memory_order_relaxed);
            break;
        default:
            break;
    }
}

void stats_record_realloc() noexcept {
    g_stats.realloc_calls.fetch_add(1, std::memory_order_relaxed);
}

void stats_record_calloc() noexcept {
    g_stats.calloc_calls.fetch_add(1, std::memory_order_relaxed);
}

void stats_record_slab_refill() noexcept {
    g_stats.slab_refills.fetch_add(1, std::memory_order_relaxed);
}

void stats_record_slab_drain() noexcept {
    g_stats.slab_drains.fetch_add(1, std::memory_order_relaxed);
}

void stats_record_slab_new() noexcept {
    g_stats.slab_new_slabs.fetch_add(1, std::memory_order_relaxed);
}

void trace_record(AllocOp op, AllocPath path, size_t size, void* ptr) noexcept {
    uint64_t seq = g_trace_seq.fetch_add(1, std::memory_order_relaxed);
    AllocTraceEvent& e = g_trace_ring[seq % TRACE_RING_SIZE];
    e.seq = seq;
    e.ptr = reinterpret_cast<uintptr_t>(ptr);
    e.size = size;
    e.op = op;
    e.path = path;
}

AllocStatsSnapshot my_malloc_stats_snapshot() noexcept {
    return AllocStatsSnapshot{
        g_stats.malloc_calls.load(std::memory_order_relaxed),
        g_stats.free_calls.load(std::memory_order_relaxed),
        g_stats.realloc_calls.load(std::memory_order_relaxed),
        g_stats.calloc_calls.load(std::memory_order_relaxed),
        g_stats.slab_allocs.load(std::memory_order_relaxed),
        g_stats.slab_frees.load(std::memory_order_relaxed),
        g_stats.slab_refills.load(std::memory_order_relaxed),
        g_stats.slab_drains.load(std::memory_order_relaxed),
        g_stats.slab_new_slabs.load(std::memory_order_relaxed),
        g_stats.tcache_allocs.load(std::memory_order_relaxed),
        g_stats.tcache_frees.load(std::memory_order_relaxed),
        g_stats.arena_allocs.load(std::memory_order_relaxed),
        g_stats.arena_frees.load(std::memory_order_relaxed),
        g_stats.mmap_frees.load(std::memory_order_relaxed),
    };
}

void my_malloc_stats_reset() noexcept {
    g_stats.malloc_calls.store(0, std::memory_order_relaxed);
    g_stats.free_calls.store(0, std::memory_order_relaxed);
    g_stats.realloc_calls.store(0, std::memory_order_relaxed);
    g_stats.calloc_calls.store(0, std::memory_order_relaxed);
    g_stats.slab_allocs.store(0, std::memory_order_relaxed);
    g_stats.slab_frees.store(0, std::memory_order_relaxed);
    g_stats.slab_refills.store(0, std::memory_order_relaxed);
    g_stats.slab_drains.store(0, std::memory_order_relaxed);
    g_stats.slab_new_slabs.store(0, std::memory_order_relaxed);
    g_stats.tcache_allocs.store(0, std::memory_order_relaxed);
    g_stats.tcache_frees.store(0, std::memory_order_relaxed);
    g_stats.arena_allocs.store(0, std::memory_order_relaxed);
    g_stats.arena_frees.store(0, std::memory_order_relaxed);
    g_stats.mmap_frees.store(0, std::memory_order_relaxed);
    g_trace_seq.store(0, std::memory_order_relaxed);
}

void my_malloc_dump_stats_json(FILE* out) noexcept {
    if (!out) return;
    AllocStatsSnapshot s = my_malloc_stats_snapshot();
    std::fprintf(out,
        "{\n"
        "  \"mode\": \"%s\",\n"
        "  \"calls\": {\"malloc\": %llu, \"free\": %llu, \"realloc\": %llu, \"calloc\": %llu},\n"
        "  \"paths\": {\"slab_alloc\": %llu, \"slab_free\": %llu, \"tcache_alloc\": %llu, \"tcache_free\": %llu, \"arena_alloc\": %llu, \"arena_free\": %llu, \"mmap_free\": %llu},\n"
        "  \"slab\": {\"refills\": %llu, \"drains\": %llu, \"new_slabs\": %llu}\n"
        "}\n",
        []() noexcept -> const char* {
            switch (allocator_mode()) {
                case AllocMode::Hybrid: return "hybrid";
                case AllocMode::PtmallocOnly: return "ptmalloc";
                case AllocMode::TcmallocLike: return "tcmalloc_like";
                case AllocMode::JemallocLike: return "jemalloc_like";
                case AllocMode::MimallocLike: return "mimalloc_like";
                case AllocMode::Adaptive: return "adaptive";
            }
            return "unknown";
        }(),
        static_cast<unsigned long long>(s.malloc_calls),
        static_cast<unsigned long long>(s.free_calls),
        static_cast<unsigned long long>(s.realloc_calls),
        static_cast<unsigned long long>(s.calloc_calls),
        static_cast<unsigned long long>(s.slab_allocs),
        static_cast<unsigned long long>(s.slab_frees),
        static_cast<unsigned long long>(s.tcache_allocs),
        static_cast<unsigned long long>(s.tcache_frees),
        static_cast<unsigned long long>(s.arena_allocs),
        static_cast<unsigned long long>(s.arena_frees),
        static_cast<unsigned long long>(s.mmap_frees),
        static_cast<unsigned long long>(s.slab_refills),
        static_cast<unsigned long long>(s.slab_drains),
        static_cast<unsigned long long>(s.slab_new_slabs));
}

} // namespace my_ptmalloc
