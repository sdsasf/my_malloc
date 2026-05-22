// OS memory primitives implementation.

#include "os.h"

#include <sys/mman.h>
#include <unistd.h>

namespace my_ptmalloc {
namespace mimalloc {

void* OS::segment_alloc() noexcept {
    // Allocate 2x segment size, trim to alignment — guarantees a 64KB-aligned
    // region of exactly MI_SEGMENT_SIZE bytes.
    size_t alloc_size = MI_SEGMENT_SIZE * 2;
    void* raw = ::mmap(nullptr, alloc_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) return nullptr;

    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t aligned  = (raw_addr + MI_SEGMENT_SIZE - 1) & ~(MI_SEGMENT_SIZE - 1);
    size_t prefix = aligned - raw_addr;
    size_t suffix = (raw_addr + alloc_size) - (aligned + MI_SEGMENT_SIZE);

    if (prefix) ::munmap(reinterpret_cast<void*>(raw_addr), prefix);
    if (suffix) ::munmap(reinterpret_cast<void*>(aligned + MI_SEGMENT_SIZE), suffix);

    return reinterpret_cast<void*>(aligned);
}

void OS::segment_free(void* seg) noexcept {
    ::munmap(seg, MI_SEGMENT_SIZE);
}

void* OS::raw_alloc(size_t size, size_t alignment) noexcept {
    if (alignment < page_size()) alignment = page_size();

    if (alignment > page_size()) {
        size_t extra = alignment;
        size_t total = size + extra;
        void* raw = ::mmap(nullptr, total, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (raw == MAP_FAILED) return nullptr;

        uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
        uintptr_t aligned  = (raw_addr + extra - 1) & ~(static_cast<uintptr_t>(extra) - 1);
        size_t front = aligned - raw_addr;
        size_t back  = (raw_addr + total) - (aligned + size);

        if (front >= page_size()) ::munmap(reinterpret_cast<void*>(raw_addr), front);
        if (back >= page_size()) ::munmap(reinterpret_cast<void*>(aligned + size), back);

        return reinterpret_cast<void*>(aligned);
    }

    size_t ps = page_size();
    size_t map_size = (size + ps - 1) & ~(ps - 1);
    void* p = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return nullptr;
    return p;
}

void OS::raw_free(void* ptr, size_t size) noexcept {
    size_t ps = page_size();
    size_t map_size = (size + ps - 1) & ~(ps - 1);
    ::munmap(ptr, map_size);
}

size_t OS::page_size() noexcept {
    static size_t ps = 0;
    if (ps == 0) {
        long p = sysconf(_SC_PAGESIZE);
        ps = (p > 0) ? static_cast<size_t>(p) : 4096;
    }
    return ps;
}

void OS::page_reset(void* addr, size_t size) noexcept {
    if (size == 0) return;
    size_t ps = page_size();
    uintptr_t a = reinterpret_cast<uintptr_t>(addr);
    uintptr_t aligned = (a + ps - 1) & ~(ps - 1);
    uintptr_t end = a + size;
    if (aligned >= end) return;
    ::madvise(reinterpret_cast<void*>(aligned), end - aligned, MADV_DONTNEED);
}

} // namespace mimalloc
} // namespace my_ptmalloc
