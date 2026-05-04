#pragma once
// LD_PRELOAD interposition: C linkage wrappers
// These functions replace the system malloc/free/etc when loaded via LD_PRELOAD

namespace my_ptmalloc {

// Bootstrap: resolve real malloc/free via dlsym
void hooks_init() noexcept;

} // namespace my_ptmalloc

// C linkage for LD_PRELOAD interposition
extern "C" {
    void* malloc(size_t size) noexcept;
    void  free(void* ptr) noexcept;
    void* calloc(size_t n, size_t size) noexcept;
    void* realloc(void* ptr, size_t size) noexcept;
    void* memalign(size_t alignment, size_t size) noexcept;
    int   posix_memalign(void** memptr, size_t alignment, size_t size) noexcept;
    void* aligned_alloc(size_t alignment, size_t size) noexcept;
    int   mallopt(int param, int value) noexcept;
    size_t malloc_usable_size(void* ptr) noexcept;
}
