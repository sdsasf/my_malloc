#include "my_ptmalloc/adaptive_allocator.h"
#include "my_ptmalloc/my_malloc.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using my_ptmalloc::AdaptiveProfileId;

static bool fail(const char* msg) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    return false;
}

static AdaptiveProfileId expected_profile() {
    const char* env = std::getenv("MY_MALLOC_ADAPTIVE_PROFILE");
    if (env && std::strcmp(env, "low_latency") == 0) return AdaptiveProfileId::LowLatency;
    if (env && std::strcmp(env, "low_rss") == 0) return AdaptiveProfileId::LowRss;
    if (env && std::strcmp(env, "large_heavy") == 0) return AdaptiveProfileId::LargeHeavy;
    if (env && std::strcmp(env, "cross_thread") == 0) return AdaptiveProfileId::CrossThread;
    return AdaptiveProfileId::Balanced;
}

int main() {
    auto state = my_ptmalloc::adaptive_control_state_snapshot();
    AdaptiveProfileId expected = expected_profile();
    if (state.control_preset != expected) return fail("control preset did not match env");

    switch (expected) {
        case AdaptiveProfileId::LowLatency:
            if (state.empty_cache_limit < 4) return fail("low_latency should keep more empty pages/spans");
            if (state.local_batch_size < 64) return fail("low_latency should raise batch/cache hint");
            break;
        case AdaptiveProfileId::LowRss:
            if (state.empty_cache_limit != 0) return fail("low_rss should aggressively release empty pages/spans");
            break;
        case AdaptiveProfileId::LargeHeavy:
            if (!state.large_path_preferred) return fail("large_heavy should prefer large/direct path");
            my_ptmalloc::adaptive_stats_reset();
            {
                void* q = my_ptmalloc::my_malloc(4096);
                if (!q) return fail("large_heavy allocation failed");
                my_ptmalloc::my_free(q);
                auto stats = my_ptmalloc::adaptive_stats_snapshot();
                if (stats.strategy[2].alloc_count == 0) {
                    return fail("large_heavy did not route medium request to large mechanism");
                }
            }
            break;
        case AdaptiveProfileId::CrossThread:
            if (!state.remote_free_reserved) return fail("cross_thread should reserve remote-free control state");
            break;
        case AdaptiveProfileId::Balanced:
            break;
    }

    void* p = my_ptmalloc::my_malloc(128);
    if (!p) return fail("adaptive allocation failed");
    my_ptmalloc::my_free(p);
    std::printf("adaptive control test: PASS\n");
    return 0;
}
