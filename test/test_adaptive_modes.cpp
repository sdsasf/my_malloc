#include "my_ptmalloc/adaptive_allocator.h"
#include "my_ptmalloc/adaptive_telemetry.h"

#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

using my_ptmalloc::ADAPTIVE_HDR_OFFSET;
using my_ptmalloc::AdaptiveHeader;
using my_ptmalloc::AdaptiveModeId;
using my_ptmalloc::AdaptiveStorageId;

static int fail(const char* msg) {
    std::fprintf(stderr, "%s\n", msg);
    return 1;
}

static AdaptiveHeader* hdr(void* p) {
    return reinterpret_cast<AdaptiveHeader*>(static_cast<char*>(p) - ADAPTIVE_HDR_OFFSET);
}

static int fixed_mode_smoke(AdaptiveModeId expected) {
    if (my_ptmalloc::adaptive_current_mode() != expected) {
        return fail("current adaptive mode does not match expected fixed mode");
    }
    void* p = my_ptmalloc::adaptive_malloc(128);
    if (!p) return fail("adaptive malloc failed");
    if (hdr(p)->mode_id != expected) return fail("header mode_id does not match allocation mode");
    if (my_ptmalloc::adaptive_usable_size(p) < 128) return fail("usable_size is too small");
    my_ptmalloc::adaptive_free(p);
    return 0;
}

static int soft_switch() {
    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::Balanced);
    void* a = my_ptmalloc::adaptive_malloc(96);
    if (!a || hdr(a)->mode_id != AdaptiveModeId::Balanced) return fail("balanced allocation failed");

    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::CompactRSS);
    void* b = my_ptmalloc::adaptive_malloc(8192);
    if (!b || hdr(b)->mode_id != AdaptiveModeId::CompactRSS) return fail("compact allocation failed");

    my_ptmalloc::adaptive_free(a);
    my_ptmalloc::adaptive_free(b);
    auto s = my_ptmalloc::adaptive_stats_snapshot();
    if (s.mode_free_count[static_cast<size_t>(AdaptiveModeId::Balanced)] == 0) {
        return fail("balanced object was not freed through allocation-time mode stats");
    }
    if (s.mode_free_count[static_cast<size_t>(AdaptiveModeId::CompactRSS)] == 0) {
        return fail("compact object was not freed through allocation-time mode stats");
    }
    return 0;
}

static int remote_selector() {
    std::vector<void*> ptrs;
    for (int i = 0; i < 256; ++i) {
        ptrs.push_back(my_ptmalloc::adaptive_malloc(64));
    }
    std::thread t([&] {
        for (void* p : ptrs) my_ptmalloc::adaptive_free(p);
    });
    t.join();
    auto after_remote = my_ptmalloc::adaptive_stats_snapshot();
    if (after_remote.remote_free_ratio <= 0.0) return fail("remote-free telemetry was not recorded");
    if (my_ptmalloc::adaptive_current_mode() != AdaptiveModeId::CrossThreadMessage) {
        return fail("remote-free window did not select cross_thread mode");
    }
    for (int i = 0; i < 256; ++i) {
        void* p = my_ptmalloc::adaptive_malloc(64);
        my_ptmalloc::adaptive_free(p);
    }
    auto s = my_ptmalloc::adaptive_stats_snapshot();
    if (s.remote_free_ratio <= 0.0) return fail("remote-free telemetry was not recorded");
    return 0;
}

static int large_selector() {
    for (int i = 0; i < 160; ++i) {
        void* p = my_ptmalloc::adaptive_malloc(512 * 1024);
        my_ptmalloc::adaptive_free(p);
    }
    if (my_ptmalloc::adaptive_current_mode() != AdaptiveModeId::LargeObjectStreaming) {
        return fail("large workload did not select large_object mode");
    }
    auto s = my_ptmalloc::adaptive_stats_snapshot();
    if (s.large_bytes_ratio <= 0.5) return fail("large bytes ratio was not recorded");
    return 0;
}

static int hardened_debug() {
    if (my_ptmalloc::adaptive_current_mode() != AdaptiveModeId::HardenedDebug) {
        return fail("debug env did not force hardened_debug mode");
    }
    void* p = my_ptmalloc::adaptive_malloc(64);
    if (!p || hdr(p)->mode_id != AdaptiveModeId::HardenedDebug) return fail("debug allocation mode mismatch");
    my_ptmalloc::adaptive_free(p);
    my_ptmalloc::adaptive_free(p);
    auto s = my_ptmalloc::adaptive_stats_snapshot();
    if (s.invalid_free_count == 0) return fail("hardened_debug did not record invalid/double free");
    return 0;
}

static int hardened_redzone() {
    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::HardenedDebug);
    void* p = my_ptmalloc::adaptive_malloc(64);
    if (!p || hdr(p)->mode_id != AdaptiveModeId::HardenedDebug) {
        return fail("hardened redzone allocation failed");
    }
    static_cast<unsigned char*>(p)[my_ptmalloc::adaptive_usable_size(p)] = 0xEE;
    my_ptmalloc::adaptive_free(p);
    auto s = my_ptmalloc::adaptive_stats_snapshot();
    if (s.header_corruption_count == 0) return fail("hardened_debug did not detect redzone corruption");
    return 0;
}

static int throughput_tcache() {
    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::ThroughputCache);
    void* p = my_ptmalloc::adaptive_malloc(64);
    if (!p) return fail("throughput tcache allocation failed");
    my_ptmalloc::adaptive_free(p);
    void* q = my_ptmalloc::adaptive_malloc(64);
    if (q != p) return fail("throughput tcache did not reuse the thread-local object");
    my_ptmalloc::adaptive_free(q);
    return 0;
}

static int cross_thread_remote_queue() {
    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::CrossThreadMessage);
    void* p = my_ptmalloc::adaptive_malloc(64);
    if (!p || hdr(p)->mode_id != AdaptiveModeId::CrossThreadMessage) {
        return fail("cross-thread allocation failed");
    }
    std::thread t([&] {
        my_ptmalloc::adaptive_free(p);
    });
    t.join();
    void* q = my_ptmalloc::adaptive_malloc(64);
    if (q != p) return fail("remote-free queue did not return object to owner thread");
    my_ptmalloc::adaptive_free(q);
    auto s = my_ptmalloc::adaptive_stats_snapshot();
    if (s.remote_free_ratio <= 0.0) return fail("remote queue test did not record remote free");
    return 0;
}

static int deterministic_latency_cache() {
    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::DeterministicLatency);
    void* p = my_ptmalloc::adaptive_malloc(64);
    if (!p || hdr(p)->mode_id != AdaptiveModeId::DeterministicLatency) {
        return fail("deterministic latency allocation failed");
    }
    my_ptmalloc::adaptive_free(p);
    void* q = my_ptmalloc::adaptive_malloc(64);
    if (q != p) return fail("deterministic latency bounded cache did not reuse the object");
    my_ptmalloc::adaptive_free(q);
    return 0;
}

static int fragmentation_stable_storage() {
    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::FragmentationStable);
    void* p = my_ptmalloc::adaptive_malloc(48 * 1024);
    if (!p || hdr(p)->mode_id != AdaptiveModeId::FragmentationStable) {
        return fail("fragmentation-stable allocation failed");
    }
    if (hdr(p)->storage != AdaptiveStorageId::Span) {
        return fail("fragmentation-stable mode did not keep medium allocation in span storage");
    }
    my_ptmalloc::adaptive_free(p);

    void* q = my_ptmalloc::adaptive_malloc(1537);
    if (!q || hdr(q)->storage != AdaptiveStorageId::Span) {
        return fail("fragmentation-stable mode did not use span storage for mixed medium allocation");
    }
    my_ptmalloc::adaptive_free(q);
    return 0;
}

static int compact_precise_reclaim() {
    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::CompactRSS);
    void* p = my_ptmalloc::adaptive_malloc(64);
    if (!p || hdr(p)->mode_id != AdaptiveModeId::CompactRSS) {
        return fail("compact reclaim allocation failed");
    }
    my_ptmalloc::adaptive_free(p);
    auto s = my_ptmalloc::adaptive_stats_snapshot();
    if (s.released_pages == 0) return fail("compact_rss did not reclaim the emptied size-class page");
    return 0;
}

static int window_delta_features() {
    my_ptmalloc::adaptive_stats_reset();
    my_ptmalloc::adaptive_set_mode(AdaptiveModeId::Balanced);
    void* p = my_ptmalloc::adaptive_malloc(64);
    if (!p) return fail("window delta allocation failed");
    my_ptmalloc::adaptive_free(p);

    auto first = my_ptmalloc::adaptive_extract_window_features();
    if (first.alloc_calls == 0 || first.requested_bytes == 0) {
        return fail("first window did not include allocation delta");
    }

    auto snapshot = my_ptmalloc::adaptive_stats_snapshot();
    if (snapshot.malloc_calls == 0) return fail("snapshot lost cumulative malloc count");

    auto second = my_ptmalloc::adaptive_extract_window_features();
    if (second.alloc_calls != 0 || second.requested_bytes != 0) {
        return fail("second window repeated allocation counters");
    }

    void* q = my_ptmalloc::adaptive_malloc(128);
    if (!q) return fail("second window allocation failed");
    my_ptmalloc::adaptive_free(q);
    auto third = my_ptmalloc::adaptive_extract_window_features();
    if (third.alloc_calls == 0 || third.requested_bytes < 128) {
        return fail("third window did not advance after new allocation");
    }
    return 0;
}

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "soft_switch";
    if (std::strcmp(mode, "balanced") == 0) return fixed_mode_smoke(AdaptiveModeId::Balanced);
    if (std::strcmp(mode, "throughput_cache") == 0) return fixed_mode_smoke(AdaptiveModeId::ThroughputCache);
    if (std::strcmp(mode, "compact_rss") == 0) return fixed_mode_smoke(AdaptiveModeId::CompactRSS);
    if (std::strcmp(mode, "large_object") == 0) return fixed_mode_smoke(AdaptiveModeId::LargeObjectStreaming);
    if (std::strcmp(mode, "hardened_debug") == 0) return hardened_debug();
    if (std::strcmp(mode, "hardened_redzone") == 0) return hardened_redzone();
    if (std::strcmp(mode, "throughput_tcache") == 0) return throughput_tcache();
    if (std::strcmp(mode, "remote_queue") == 0) return cross_thread_remote_queue();
    if (std::strcmp(mode, "deterministic_cache") == 0) return deterministic_latency_cache();
    if (std::strcmp(mode, "fragmentation_storage") == 0) return fragmentation_stable_storage();
    if (std::strcmp(mode, "compact_reclaim") == 0) return compact_precise_reclaim();
    if (std::strcmp(mode, "window_delta") == 0) return window_delta_features();
    if (std::strcmp(mode, "soft_switch") == 0) return soft_switch();
    if (std::strcmp(mode, "remote_selector") == 0) return remote_selector();
    if (std::strcmp(mode, "large_selector") == 0) return large_selector();
    return fail("unknown adaptive mode test");
}
