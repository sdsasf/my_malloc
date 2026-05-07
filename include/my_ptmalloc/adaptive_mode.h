#pragma once
// Adaptive Mode Policy Layer. Modes are policy descriptions over shared
// memory services; they never directly mutate page/span/direct-map internals.

#include "my_ptmalloc/adaptive_services.h"

namespace my_ptmalloc {

struct AllocationPlan {
    StoragePreference storage;
    bool use_thread_cache;
    bool prefer_reuse;
    bool prefer_low_rss;
    bool debug_redzone;
    bool debug_quarantine;
    bool prefer_direct_map;
    uint32_t batch_size;
    uint32_t empty_keep_limit;
};

struct AdaptiveModePolicy {
    AdaptiveModeId id;
    const char* name;
    const char* objective;
    AllocationPlan (*plan_allocate)(size_t size, size_t alignment) noexcept;
    ReleaseDecision (*plan_free)(AdaptiveHeader* hdr) noexcept;
    void (*on_activate)() noexcept;
    void (*on_retire)() noexcept;
    void (*on_window)() noexcept;
};

const AdaptiveModePolicy& adaptive_mode_policy(AdaptiveModeId id) noexcept;
const char* adaptive_mode_name(AdaptiveModeId id) noexcept;
const char* adaptive_mode_objective(AdaptiveModeId id) noexcept;

} // namespace my_ptmalloc
