#pragma once
// SysMemory: abstract interface for system memory allocation
// Enables testing with mock memory sources

#include <cstddef>

namespace my_ptmalloc {

class SysMemory {
public:
    virtual ~SysMemory() = default;

    // Allocate size bytes with given alignment
    virtual void* map(size_t size, size_t alignment) noexcept = 0;

    // Release previously allocated memory
    virtual void unmap(void* addr, size_t size) noexcept = 0;

    // Try to grow allocation in-place (returns true on success)
    virtual bool grow_inplace(void* addr, size_t old_size, size_t new_size) noexcept = 0;
};

// Real implementation using mmap/munmap
class MmapMemory : public SysMemory {
public:
    void* map(size_t size, size_t alignment) noexcept override;
    void unmap(void* addr, size_t size) noexcept override;
    bool grow_inplace(void* addr, size_t old_size, size_t new_size) noexcept override;
};

// Fixed pool for testing
class PoolMemory : public SysMemory {
    void*  base_;
    size_t capacity_;
    size_t offset_;

public:
    PoolMemory(void* base, size_t capacity);
    void* map(size_t size, size_t alignment) noexcept override;
    void unmap(void* addr, size_t size) noexcept override;
    bool grow_inplace(void* addr, size_t old_size, size_t new_size) noexcept override;
};

} // namespace my_ptmalloc
