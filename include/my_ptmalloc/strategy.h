#pragma once
// Pluggable allocator strategy API — shared between adaptive and classic allocators.

#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {

constexpr uint32_t STRATEGY_API_VERSION = 1;

struct StrategyStats {
    uint64_t alloc_calls;
    uint64_t free_calls;
    uint64_t realloc_calls;
    uint64_t usable_size_calls;
};

struct StrategyVTable {
    void (*init)() noexcept;
    void (*shutdown)() noexcept;
    void* (*allocate)(size_t size) noexcept;
    void (*deallocate)(void* ptr) noexcept;
    void* (*reallocate)(void* ptr, size_t size) noexcept;
    size_t (*usable_size)(void* ptr) noexcept;
    StrategyStats (*stats)() noexcept;
};

struct StrategyDescriptor {
    uint32_t api_version;
    const char* name;
    const char* description;
    StrategyVTable vtable;
};

using StrategyEntryFn = StrategyDescriptor (*)() noexcept;

// Built-in strategies
StrategyDescriptor adaptive_strategy_descriptor() noexcept;
StrategyDescriptor libc_strategy_descriptor() noexcept;

// Classic allocator reproductions (defined in classic/ subdirectories)
StrategyDescriptor ptmalloc_classic_strategy_descriptor() noexcept;
StrategyDescriptor tcmalloc_classic_strategy_descriptor() noexcept;
StrategyDescriptor jemalloc_classic_strategy_descriptor() noexcept;
StrategyDescriptor mimalloc_classic_strategy_descriptor() noexcept;

} // namespace my_ptmalloc
