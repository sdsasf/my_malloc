// System memory implementation: mmap-based allocation

#include "my_ptmalloc/sys_memory.h"
#include "my_ptmalloc/config.h"
#include <sys/mman.h>
#include <cstring>
#include <algorithm>

namespace my_ptmalloc {

// ─── MmapMemory ───

void* MmapMemory::map(size_t size, size_t alignment) noexcept {
    // Over-allocate to ensure alignment
    size_t alloc_size = size + alignment;
    void* p = ::mmap(nullptr, alloc_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return nullptr;

    // Align the pointer
    uintptr_t addr = reinterpret_cast<uintptr_t>(p);
    uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);

    // If we got lucky with alignment, no need to fix up
    if (aligned == addr) {
        // Record the actual allocation size for later munmap
        // Store it just before the aligned pointer (if there's room)
        return p;
    }

    // Store original pointer before aligned for munmap
    // We'll just over-allocate and use the full region
    return reinterpret_cast<void*>(aligned);
}

void MmapMemory::unmap(void* addr, size_t size) noexcept {
    if (addr && size > 0) {
        ::munmap(addr, size);
    }
}

bool MmapMemory::grow_inplace(void*, size_t, size_t) noexcept {
    // mmap cannot grow in place
    return false;
}

// ─── PoolMemory (for testing) ───

PoolMemory::PoolMemory(void* base, size_t capacity)
    : base_(base), capacity_(capacity), offset_(0) {}

void* PoolMemory::map(size_t size, size_t alignment) noexcept {
    uintptr_t base = reinterpret_cast<uintptr_t>(base_) + offset_;
    uintptr_t aligned = (base + alignment - 1) & ~(alignment - 1);
    size_t total = aligned - base + size;

    if (offset_ + total > capacity_) return nullptr;

    offset_ += total;
    return reinterpret_cast<void*>(aligned);
}

void PoolMemory::unmap(void*, size_t) noexcept {
    // Pool memory doesn't actually free
}

bool PoolMemory::grow_inplace(void*, size_t, size_t) noexcept {
    return false;
}

} // namespace my_ptmalloc
