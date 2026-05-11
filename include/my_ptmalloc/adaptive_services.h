#pragma once
// Shared Memory Management Layer interface. Adaptive modes express intent
// through AllocationRequest and ReleaseDecision; this layer owns concrete
// size-class pages, spans, direct mappings, ownership lookup, and reclaim.

#include "my_ptmalloc/adaptive_types.h"

namespace my_ptmalloc {

enum class StoragePreference : uint8_t {
    Auto,
    SizeClass,
    Span,
    Extent,
    DirectMap,
};

enum class ReleaseAction : uint8_t {
    Cache,
    ReturnToCentralPool,
    Purge,
    Unmap,
    Quarantine,
};

struct AllocationRequest {
    size_t size;
    size_t alignment;
    AdaptiveModeId mode;
    uint64_t thread_id;

    bool zero_fill;
    bool prefer_thread_cache;
    bool prefer_direct_map;
    bool prefer_reuse;
    bool prefer_low_rss;
    bool debug_redzone;
    bool debug_quarantine;
    bool drain_remote_queue;
    bool prefer_occupancy_packing;

    uint32_t batch_size;
    uint32_t empty_keep_limit;
    uint32_t tcache_small_limit;
    uint32_t tcache_medium_limit;
    size_t direct_map_threshold;
};

struct AllocationResult {
    void* user_ptr;
    AdaptiveHeader* header;
    AdaptiveStorageId storage;
    size_t requested;
    size_t usable;
    size_t mapped_size;
    size_t newly_mapped_size;
    bool from_cache;
    bool used_mmap;
    bool slow_path;
};

struct ReleaseDecision {
    ReleaseAction action;
    bool poison;
    bool check_redzone;
    bool quarantine;
    bool use_remote_queue;
    uint32_t empty_keep_limit;
    uint32_t tcache_small_limit;
    uint32_t tcache_medium_limit;
};

struct MemoryServices {
    AllocationResult allocate_auto(const AllocationRequest& req) noexcept;
    AllocationResult allocate_size_class(const AllocationRequest& req) noexcept;
    AllocationResult allocate_span(const AllocationRequest& req) noexcept;
    AllocationResult allocate_extent(const AllocationRequest& req) noexcept;
    AllocationResult allocate_direct_map(const AllocationRequest& req) noexcept;

    void deallocate(AdaptiveHeader* hdr, const ReleaseDecision& decision) noexcept;
    void* reallocate(AdaptiveHeader* hdr, size_t new_size) noexcept;
    size_t usable_size(AdaptiveHeader* hdr) noexcept;

    bool owns(void* ptr) noexcept;
    AdaptiveHeader* header_from_user(void* ptr) noexcept;
};

MemoryServices& adaptive_memory_services() noexcept;

} // namespace my_ptmalloc
