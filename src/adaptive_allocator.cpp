// Adaptive allocator public facade. This file intentionally contains no
// page/span/direct-map implementation and no mode-selection rules.

#include "my_ptmalloc/adaptive_allocator.h"
#include "my_ptmalloc/adaptive_mode.h"
#include "my_ptmalloc/adaptive_runtime.h"
#include "my_ptmalloc/adaptive_selector.h"
#include "my_ptmalloc/adaptive_services.h"
#include "my_ptmalloc/adaptive_telemetry.h"

#include <cstring>

namespace my_ptmalloc {

namespace {

static AllocationRequest request_from_plan(size_t size,
                                           size_t alignment,
                                           AdaptiveModeId mode,
                                           const AllocationPlan& plan) noexcept {
    return AllocationRequest{
        size,
        alignment,
        mode,
        adaptive_thread_token(),
        false,
        plan.use_thread_cache,
        plan.prefer_direct_map || plan.storage == StoragePreference::DirectMap,
        plan.prefer_reuse,
        plan.prefer_low_rss,
        plan.debug_redzone,
        plan.debug_quarantine,
        plan.batch_size,
        plan.empty_keep_limit,
    };
}

static AllocationResult allocate_with_active_mode(size_t size, size_t alignment) noexcept {
    adaptive_runtime_init();
    AdaptiveModeId mode = adaptive_current_mode();
    const AdaptiveModePolicy& policy = adaptive_mode_policy(mode);
    AllocationPlan plan = policy.plan_allocate(size, alignment);
    AllocationRequest req = request_from_plan(size, alignment, mode, plan);

    MemoryServices& services = adaptive_memory_services();
    AllocationResult result{};
    switch (plan.storage) {
        case StoragePreference::SizeClass:
            result = services.allocate_size_class(req);
            break;
        case StoragePreference::Span:
            result = services.allocate_span(req);
            break;
        case StoragePreference::Extent:
            result = services.allocate_extent(req);
            break;
        case StoragePreference::DirectMap:
            result = services.allocate_direct_map(req);
            break;
        case StoragePreference::Auto:
            result = services.allocate_auto(req);
            break;
    }
    if (result.user_ptr) {
        adaptive_telemetry_on_alloc(result);
    } else {
        adaptive_telemetry_on_alloc_failure();
    }
    adaptive_selector_maybe_switch();
    return result;
}

} // namespace

void* adaptive_malloc(size_t size) noexcept {
    adaptive_telemetry_on_malloc_call();
    return allocate_with_active_mode(size, 16).user_ptr;
}

void adaptive_free(void* ptr) noexcept {
    if (!ptr) return;
    adaptive_telemetry_on_free_call();

    MemoryServices& services = adaptive_memory_services();
    AdaptiveHeader* hdr = services.header_from_user(ptr);
    if (!hdr) {
        adaptive_telemetry_on_invalid_free(ptr);
        adaptive_selector_maybe_switch();
        return;
    }

    const AdaptiveModePolicy& policy = adaptive_mode_policy(hdr->mode_id);
    ReleaseDecision decision = policy.plan_free(hdr);
    adaptive_telemetry_on_free_begin(hdr);
    services.deallocate(hdr, decision);
    adaptive_telemetry_on_free_end(hdr);
    adaptive_selector_maybe_switch();
}

void* adaptive_realloc(void* ptr, size_t size) noexcept {
    adaptive_telemetry_on_realloc();
    if (!ptr) return adaptive_malloc(size);
    if (size == 0) {
        adaptive_free(ptr);
        return nullptr;
    }

    MemoryServices& services = adaptive_memory_services();
    AdaptiveHeader* hdr = services.header_from_user(ptr);
    if (!hdr) {
        adaptive_telemetry_on_invalid_free(ptr);
        return nullptr;
    }
    return services.reallocate(hdr, size);
}

size_t adaptive_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;
    MemoryServices& services = adaptive_memory_services();
    AdaptiveHeader* hdr = services.header_from_user(ptr);
    if (!hdr) return 0;
    return adaptive_mode_policy(hdr->mode_id).id == hdr->mode_id
        ? services.usable_size(hdr)
        : 0;
}

void* adaptive_memalign(size_t alignment, size_t size) noexcept {
    if (alignment < sizeof(void*)) alignment = sizeof(void*);
    if (alignment <= 16) return adaptive_malloc(size);
    if (alignment & (alignment - 1)) {
        alignment--;
        alignment |= alignment >> 1;
        alignment |= alignment >> 2;
        alignment |= alignment >> 4;
        alignment |= alignment >> 8;
        alignment |= alignment >> 16;
        alignment |= alignment >> 32;
        alignment++;
        if (alignment == 0) {
            adaptive_telemetry_on_alloc_failure();
            return nullptr;
        }
    }
    adaptive_telemetry_on_malloc_call();
    return allocate_with_active_mode(size, alignment).user_ptr;
}

bool adaptive_owns(void* ptr) noexcept {
    return adaptive_memory_services().owns(ptr);
}

} // namespace my_ptmalloc
