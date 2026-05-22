// OS-level memory allocation implementation.

#include "sys_memory.h"

#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>

namespace my_ptmalloc {
namespace ptmalloc {

size_t SysMemory::page_size() noexcept {
    static size_t ps = 0;
    if (ps == 0) {
        long p = sysconf(_SC_PAGESIZE);
        ps = (p > 0) ? static_cast<size_t>(p) : 4096;
    }
    return ps;
}

void* SysMemory::map(size_t size, size_t alignment) noexcept {
    // Round up to page size
    size_t ps = page_size();
    size_t map_size = (size + ps - 1) & ~(ps - 1);

    // For large alignments, over-allocate and trim
    if (alignment > ps) {
        size_t extra = alignment;
        map_size = map_size + extra;
        void* raw = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (raw == MAP_FAILED) return nullptr;

        uintptr_t aligned = (reinterpret_cast<uintptr_t>(raw) + extra - 1)
                           & ~(static_cast<uintptr_t>(extra) - 1);
        size_t prefix = aligned - reinterpret_cast<uintptr_t>(raw);
        size_t suffix = (reinterpret_cast<uintptr_t>(raw) + map_size)
                      - (aligned + size);

        if (prefix >= ps) ::munmap(raw, prefix);
        if (suffix >= ps) ::munmap(reinterpret_cast<void*>(aligned + size), suffix);
        return reinterpret_cast<void*>(aligned);
    }

    void* p = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return nullptr;
    return p;
}

void SysMemory::unmap(Chunk* p, size_t size) noexcept {
    size_t ps = page_size();
    // For mmap'd chunks, the chunk starts at the mapped region.
    // Round size up, but don't unmap below p.
    size_t map_size = (size + ps - 1) & ~(ps - 1);
    ::munmap(static_cast<void*>(p), map_size);
}

void SysMemory::unmap_region(void* addr, size_t size) noexcept {
    if (size == 0) return;
    ::munmap(addr, size);
}

bool SysMemory::trim(void* start, size_t current_size, size_t target_size,
                     size_t extra_pad) noexcept {
    size_t ps = page_size();
    size_t keep_size = target_size + extra_pad;
    if (keep_size >= current_size) return false;

    // Calculate how much to release (page-aligned trailing region)
    uintptr_t keep_end = reinterpret_cast<uintptr_t>(start) + keep_size;
    uintptr_t cur_end  = reinterpret_cast<uintptr_t>(start) + current_size;

    uintptr_t release_start = (keep_end + ps - 1) & ~(ps - 1);
    if (release_start >= cur_end) return false;

    size_t release_size = cur_end - release_start;
    if (release_size < ps) return false;

    // Try MADV_DONTNEED first (preserves virtual address space, releases physical)
    int ret = ::madvise(reinterpret_cast<void*>(release_start), release_size,
                        MADV_DONTNEED);
    if (ret == 0) return true;

    // Fall back to munmap partial
    ::munmap(reinterpret_cast<void*>(release_start), release_size);
    return true;
}

} // namespace ptmalloc
} // namespace my_ptmalloc
