// LD_PRELOAD C linkage hooks
// These replace system malloc/free when loaded via LD_PRELOAD

#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/thread_registry.h"
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>

// Real malloc/free from libc (resolved via dlsym)
static void* (*real_malloc)(size_t) = nullptr;
static void  (*real_free)(void*) = nullptr;
static void* (*real_calloc)(size_t, size_t) = nullptr;
static void* (*real_realloc)(void*, size_t) = nullptr;

static bool hooks_initialized = false;

namespace my_ptmalloc {

void hooks_init() noexcept {
    if (hooks_initialized) return;
    hooks_initialized = true;

    real_malloc  = reinterpret_cast<void*(*)(size_t)>(dlsym(RTLD_NEXT, "malloc"));
    real_free    = reinterpret_cast<void(*)(void*)>(dlsym(RTLD_NEXT, "free"));
    real_calloc  = reinterpret_cast<void*(*)(size_t, size_t)>(dlsym(RTLD_NEXT, "calloc"));
    real_realloc = reinterpret_cast<void*(*)(void*, size_t)>(dlsym(RTLD_NEXT, "realloc"));
}

} // namespace my_ptmalloc

// Bootstrap buffer for allocations before hooks are ready
static char bootstrap_buf[4096];
static size_t bootstrap_offset = 0;
static bool bootstrap_active = true;

constexpr size_t REAL_BOOTSTRAP_MAX = 64;
static void* real_bootstrap_ptrs[REAL_BOOTSTRAP_MAX];

static void real_bootstrap_track(void* ptr) noexcept {
    if (!ptr) return;
    for (size_t i = 0; i < REAL_BOOTSTRAP_MAX; ++i) {
        if (real_bootstrap_ptrs[i] == nullptr) {
            real_bootstrap_ptrs[i] = ptr;
            return;
        }
    }
}

static bool real_bootstrap_untrack(void* ptr) noexcept {
    if (!ptr) return false;
    for (size_t i = 0; i < REAL_BOOTSTRAP_MAX; ++i) {
        if (real_bootstrap_ptrs[i] == ptr) {
            real_bootstrap_ptrs[i] = nullptr;
            return true;
        }
    }
    return false;
}

static bool real_bootstrap_replace(void* old_ptr, void* new_ptr) noexcept {
    if (!old_ptr) {
        real_bootstrap_track(new_ptr);
        return true;
    }
    for (size_t i = 0; i < REAL_BOOTSTRAP_MAX; ++i) {
        if (real_bootstrap_ptrs[i] == old_ptr) {
            real_bootstrap_ptrs[i] = new_ptr;
            return true;
        }
    }
    return false;
}

static void* bootstrap_malloc(size_t size) noexcept {
    size = (size + 15) & ~15;  // align to 16
    if (bootstrap_offset + size > sizeof(bootstrap_buf)) {
        return nullptr;  // Bootstrap buffer exhausted
    }
    void* p = bootstrap_buf + bootstrap_offset;
    bootstrap_offset += size;
    return p;
}


// ─── C linkage hooks ───

extern "C" {

void* malloc(size_t size) noexcept {
    if (!hooks_initialized) {
        my_ptmalloc::hooks_init();
        // Use real malloc for bootstrap
        if (real_malloc) {
            bootstrap_active = false;
            void* p = real_malloc(size);
            real_bootstrap_track(p);
            return p;
        }
        return bootstrap_malloc(size);
    }
    if (bootstrap_active) {
        return bootstrap_malloc(size);
    }
    return my_ptmalloc::my_malloc(size);
}

void free(void* ptr) noexcept {
    if (!ptr) return;

    // Check if ptr is in bootstrap buffer
    if (ptr >= (void*)bootstrap_buf &&
        ptr < (void*)(bootstrap_buf + sizeof(bootstrap_buf))) {
        return;  // Bootstrap allocation, no-op
    }

    if (!hooks_initialized || bootstrap_active) {
        if (real_free) real_free(ptr);
        return;
    }

    if (real_bootstrap_untrack(ptr)) {
        if (real_free) real_free(ptr);
        return;
    }

    my_ptmalloc::my_free(ptr);
}

void* calloc(size_t n, size_t size) noexcept {
    if (!hooks_initialized) {
        my_ptmalloc::hooks_init();
        if (real_calloc) {
            bootstrap_active = false;
            void* p = real_calloc(n, size);
            real_bootstrap_track(p);
            return p;
        }
        void* p = bootstrap_malloc(n * size);
        if (p) memset(p, 0, n * size);
        return p;
    }
    if (bootstrap_active) {
        void* p = bootstrap_malloc(n * size);
        if (p) memset(p, 0, n * size);
        return p;
    }
    return my_ptmalloc::my_calloc(n, size);
}

void* realloc(void* ptr, size_t size) noexcept {
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return nullptr; }

    // Check if ptr is in bootstrap buffer
    if (ptr >= (void*)bootstrap_buf &&
        ptr < (void*)(bootstrap_buf + sizeof(bootstrap_buf))) {
        void* new_ptr = malloc(size);
        if (new_ptr) {
            // Copy what we can (we don't know old size in bootstrap)
            memcpy(new_ptr, ptr, size);
        }
        return new_ptr;
    }

    if (!hooks_initialized || bootstrap_active) {
        if (real_realloc) return real_realloc(ptr, size);
        return nullptr;
    }

    if (real_bootstrap_replace(ptr, ptr)) {
        if (!real_realloc) return nullptr;
        void* new_ptr = real_realloc(ptr, size);
        if (new_ptr) real_bootstrap_replace(ptr, new_ptr);
        return new_ptr;
    }

    return my_ptmalloc::my_realloc(ptr, size);
}

void* memalign(size_t alignment, size_t size) noexcept {
    return my_ptmalloc::my_memalign(alignment, size);
}

int posix_memalign(void** memptr, size_t alignment, size_t size) noexcept {
    return my_ptmalloc::my_posix_memalign(memptr, alignment, size);
}

void* aligned_alloc(size_t alignment, size_t size) noexcept {
    return my_ptmalloc::my_aligned_alloc(alignment, size);
}

int mallopt(int param, int value) noexcept {
    return my_ptmalloc::my_mallopt(param, value);
}

size_t malloc_usable_size(void* ptr) noexcept {
    return my_ptmalloc::my_malloc_usable_size(ptr);
}

} // extern "C"
