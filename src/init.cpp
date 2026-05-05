// Global initialization

#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/coalesce.h"
#include "my_ptmalloc/threshold.h"
#include "my_ptmalloc/observer.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/alloc_pipeline.h"
#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/allocator_lab.h"
#include <pthread.h>

namespace my_ptmalloc {

// ─── Global singletons ───

Arena main_arena;
EagerCoalesce    g_eager_coalesce;
StaticThreshold  g_static_threshold;
NullObserver     g_null_observer;

CoalescePolicy*   g_coalesce_policy   = &g_eager_coalesce;
ThresholdPolicy*  g_threshold_policy  = &g_static_threshold;
AllocObserver*    g_observer          = &g_null_observer;
ArenaManager*     g_arena_manager     = nullptr;
AllocPipeline*    g_alloc_pipeline    = nullptr;

// ─── Init ───

static bool g_initialized = false;

void my_malloc_init() noexcept {
    if (g_initialized) return;
    g_initialized = true;
    allocator_lab_init();

    // Create arena manager (initializes main arena and top chunk)
    static ArenaManager arena_manager_storage;
    g_arena_manager = &arena_manager_storage;

    // Create allocation pipeline
    static AllocPipeline pipeline_storage;
    g_alloc_pipeline = &pipeline_storage;
}

// ─── Arena implementation ───

void Arena::init(bool is_main) noexcept {
    mutex_ = PTHREAD_MUTEX_INITIALIZER;
    flags_ = 0;
    top_ = nullptr;
    last_remainder_ = nullptr;
    next_ = nullptr;
    next_free_ = nullptr;
    attached_threads_ = is_main ? 1 : 0;
    system_mem_ = 0;
    max_system_mem_ = 0;
    bins_.init();
}

// ─── ArenaGuard implementation ───

ArenaGuard::ArenaGuard(Arena* av) noexcept : arena_(av) {
    if (arena_) arena_->lock();
}

ArenaGuard::~ArenaGuard() noexcept {
    if (arena_) arena_->unlock();
}

} // namespace my_ptmalloc
