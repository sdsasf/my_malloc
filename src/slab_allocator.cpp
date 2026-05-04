// Small-object slab allocator.

#include "my_ptmalloc/slab_allocator.h"
#include "my_ptmalloc/config.h"

#include <pthread.h>
#include <sys/mman.h>
#include <atomic>
#include <cstdint>
#include <cstring>

namespace my_ptmalloc {

namespace {

constexpr size_t SLAB_ALIGNMENT = 64 * 1024;
constexpr size_t SLAB_CLASS_STEP = 16;
constexpr size_t SLAB_CLASSES = SLAB_MAX_ALLOC / SLAB_CLASS_STEP;
constexpr size_t SLAB_TABLE_SIZE = 16384;
constexpr uint64_t SLAB_MAGIC = 0x6d795f736c616231ULL; // "my_slab1"

struct FreeObj {
    FreeObj* next;
};

struct alignas(64) SlabHeader {
    uint64_t magic;
    uint16_t class_idx;
    uint16_t object_size;
    uint32_t object_count;
    void* base;
};

struct SlabTableEntry {
    std::atomic<void*> base;
    std::atomic<SlabHeader*> slab;
};

thread_local FreeObj* tls_lists[SLAB_CLASSES]{};
thread_local bool slab_bypass = false;

pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
SlabTableEntry slab_table[SLAB_TABLE_SIZE]{};

[[nodiscard]] inline size_t class_index(size_t size) noexcept {
    if (size == 0) size = 1;
    return (size + SLAB_CLASS_STEP - 1) / SLAB_CLASS_STEP - 1;
}

[[nodiscard]] inline size_t class_size(size_t idx) noexcept {
    return (idx + 1) * SLAB_CLASS_STEP;
}

[[nodiscard]] inline uintptr_t slab_base_for(void* ptr) noexcept {
    return reinterpret_cast<uintptr_t>(ptr) & ~(SLAB_ALIGNMENT - 1);
}

[[nodiscard]] inline size_t table_index(void* base) noexcept {
    return (reinterpret_cast<uintptr_t>(base) >> 16) & (SLAB_TABLE_SIZE - 1);
}

void table_insert(void* base, SlabHeader* slab) noexcept {
    pthread_mutex_lock(&table_lock);
    size_t idx = table_index(base);
    for (size_t n = 0; n < SLAB_TABLE_SIZE; ++n) {
        SlabTableEntry& e = slab_table[(idx + n) & (SLAB_TABLE_SIZE - 1)];
        void* cur = e.base.load(std::memory_order_acquire);
        if (cur == nullptr || cur == base) {
            e.slab.store(slab, std::memory_order_release);
            e.base.store(base, std::memory_order_release);
            break;
        }
    }
    pthread_mutex_unlock(&table_lock);
}

[[nodiscard]] SlabHeader* table_lookup(void* ptr) noexcept {
    void* base = reinterpret_cast<void*>(slab_base_for(ptr));
    size_t idx = table_index(base);
    SlabHeader* result = nullptr;
    for (size_t n = 0; n < SLAB_TABLE_SIZE; ++n) {
        SlabTableEntry& e = slab_table[(idx + n) & (SLAB_TABLE_SIZE - 1)];
        void* cur = e.base.load(std::memory_order_acquire);
        if (cur == nullptr) break;
        if (cur == base) {
            result = e.slab.load(std::memory_order_acquire);
            break;
        }
    }
    if (!result || result->magic != SLAB_MAGIC) return nullptr;
    return result;
}

[[nodiscard]] SlabHeader* allocate_slab(size_t idx) noexcept {
    size_t map_size = SLAB_ALIGNMENT * 2;
    void* raw = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) return nullptr;

    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t aligned = (raw_addr + SLAB_ALIGNMENT - 1) & ~(SLAB_ALIGNMENT - 1);
    size_t prefix = aligned - raw_addr;
    size_t suffix = (raw_addr + map_size) - (aligned + SLAB_ALIGNMENT);
    if (prefix) ::munmap(reinterpret_cast<void*>(raw_addr), prefix);
    if (suffix) ::munmap(reinterpret_cast<void*>(aligned + SLAB_ALIGNMENT), suffix);

    auto* slab = reinterpret_cast<SlabHeader*>(aligned);
    slab->magic = SLAB_MAGIC;
    slab->class_idx = static_cast<uint16_t>(idx);
    slab->object_size = static_cast<uint16_t>(class_size(idx));
    slab->base = reinterpret_cast<void*>(aligned);

    uintptr_t start = aligned + sizeof(SlabHeader);
    start = (start + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;
    size_t usable = SLAB_ALIGNMENT - (start - aligned);
    slab->object_count = static_cast<uint32_t>(usable / slab->object_size);

    FreeObj* head = nullptr;
    for (size_t i = 0; i < slab->object_count; ++i) {
        auto* obj = reinterpret_cast<FreeObj*>(start + i * slab->object_size);
        obj->next = head;
        head = obj;
    }
    tls_lists[idx] = head;

    table_insert(reinterpret_cast<void*>(aligned), slab);
    return slab;
}

} // namespace

ScopedSlabBypass::ScopedSlabBypass() noexcept : old_(slab_bypass) {
    slab_bypass = true;
}

ScopedSlabBypass::~ScopedSlabBypass() noexcept {
    slab_bypass = old_;
}

void* slab_malloc(size_t size) noexcept {
    if (slab_bypass || size > SLAB_MAX_ALLOC) return nullptr;
    size_t idx = class_index(size);
    if (idx >= SLAB_CLASSES) return nullptr;

    FreeObj* head = tls_lists[idx];
    if (!head) {
        if (!allocate_slab(idx)) return nullptr;
        head = tls_lists[idx];
        if (!head) return nullptr;
    }

    tls_lists[idx] = head->next;
    return head;
}

bool slab_free(void* ptr) noexcept {
    SlabHeader* slab = table_lookup(ptr);
    if (!slab) return false;
    size_t idx = slab->class_idx;
    if (idx >= SLAB_CLASSES) return false;

    auto* obj = static_cast<FreeObj*>(ptr);
    obj->next = tls_lists[idx];
    tls_lists[idx] = obj;
    return true;
}

bool slab_contains(void* ptr) noexcept {
    return table_lookup(ptr) != nullptr;
}

size_t slab_usable_size(void* ptr) noexcept {
    SlabHeader* slab = table_lookup(ptr);
    return slab ? slab->object_size : 0;
}

} // namespace my_ptmalloc
