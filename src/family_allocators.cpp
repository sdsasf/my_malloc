// Teaching allocator-family implementations.

#include "my_ptmalloc/family_allocators.h"
#include "my_ptmalloc/config.h"

#include <pthread.h>
#include <sys/mman.h>
#include <atomic>
#include <cstdint>
#include <cstring>

namespace my_ptmalloc {

namespace {

constexpr size_t PAGE_SIZE = 64 * 1024;
constexpr size_t CLASS_STEP = 16;
constexpr size_t MAX_SMALL = 4096;
constexpr size_t CLASS_COUNT = MAX_SMALL / CLASS_STEP;
constexpr size_t TABLE_SIZE = 32768;
constexpr size_t LARGE_TABLE_SIZE = 4096;
constexpr size_t REFILL_BATCH = 32;
constexpr size_t DRAIN_BATCH = 64;
constexpr size_t LOCAL_LIMIT = 128;
constexpr size_t JEMALLOC_ARENAS = 8;
constexpr uint64_t PAGE_MAGIC = 0x6d616c6c6f635031ULL;  // "mallocP1"
constexpr uint64_t LARGE_MAGIC = 0x6d616c6c6f634c31ULL; // "mallocL1"

enum class Family : uint8_t {
    TcmallocLike = 1,
    JemallocLike = 2,
    MimallocLike = 3,
};

struct FreeObj {
    FreeObj* next;
};

struct PageHeader {
    uint64_t magic;
    Family family;
    uint16_t class_idx;
    uint16_t object_size;
    uint32_t object_count;
    uint32_t owner_heap;
    std::atomic<FreeObj*> remote_free;
    FreeObj* local_free;
    size_t local_count;
    PageHeader* next_page;
};

struct LargeHeader {
    uint64_t magic;
    void* mapping;
    size_t mapping_size;
    size_t usable_size;
};

struct PageTableEntry {
    std::atomic<void*> base;
    std::atomic<PageHeader*> page;
};

struct LargeTableEntry {
    std::atomic<void*> ptr;
    std::atomic<LargeHeader*> header;
};

struct alignas(64) CentralClass {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    FreeObj* list = nullptr;
    size_t count = 0;
};

struct alignas(64) JemallocArenaClass {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    PageHeader* pages = nullptr;
};

struct alignas(64) JemallocArena {
    JemallocArenaClass classes[CLASS_COUNT];
};

thread_local FreeObj* tc_local[CLASS_COUNT]{};
thread_local size_t tc_count[CLASS_COUNT]{};

thread_local FreeObj* je_tcache[CLASS_COUNT]{};
thread_local size_t je_tcache_count[CLASS_COUNT]{};
thread_local size_t je_arena_index = SIZE_MAX;

thread_local FreeObj* mi_local[CLASS_COUNT]{};
thread_local size_t mi_count[CLASS_COUNT]{};
thread_local PageHeader* mi_owned_pages[CLASS_COUNT]{};
thread_local uint32_t mi_heap_id = 0;

std::atomic<uint32_t> next_heap_id{1};
std::atomic<size_t> next_arena{0};
pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
PageTableEntry page_table[TABLE_SIZE]{};
pthread_mutex_t large_table_lock = PTHREAD_MUTEX_INITIALIZER;
LargeTableEntry large_table[LARGE_TABLE_SIZE]{};
CentralClass tc_central[CLASS_COUNT]{};
JemallocArena je_arenas[JEMALLOC_ARENAS]{};

[[nodiscard]] inline size_t class_index(size_t size) noexcept {
    if (size == 0) size = 1;
    return (size + CLASS_STEP - 1) / CLASS_STEP - 1;
}

[[nodiscard]] inline size_t class_size(size_t idx) noexcept {
    return (idx + 1) * CLASS_STEP;
}

[[nodiscard]] inline uintptr_t page_base_for(void* ptr) noexcept {
    return reinterpret_cast<uintptr_t>(ptr) & ~(PAGE_SIZE - 1);
}

[[nodiscard]] inline size_t table_index(void* base) noexcept {
    return (reinterpret_cast<uintptr_t>(base) >> 16) & (TABLE_SIZE - 1);
}

void table_insert(void* base, PageHeader* page) noexcept {
    pthread_mutex_lock(&table_lock);
    size_t idx = table_index(base);
    for (size_t n = 0; n < TABLE_SIZE; ++n) {
        PageTableEntry& e = page_table[(idx + n) & (TABLE_SIZE - 1)];
        void* cur = e.base.load(std::memory_order_acquire);
        if (!cur || cur == base) {
            e.page.store(page, std::memory_order_release);
            e.base.store(base, std::memory_order_release);
            break;
        }
    }
    pthread_mutex_unlock(&table_lock);
}

[[nodiscard]] PageHeader* page_lookup(void* ptr) noexcept {
    void* base = reinterpret_cast<void*>(page_base_for(ptr));
    size_t idx = table_index(base);
    for (size_t n = 0; n < TABLE_SIZE; ++n) {
        PageTableEntry& e = page_table[(idx + n) & (TABLE_SIZE - 1)];
        void* cur = e.base.load(std::memory_order_acquire);
        if (!cur) return nullptr;
        if (cur == base) {
            PageHeader* page = e.page.load(std::memory_order_acquire);
            return page && page->magic == PAGE_MAGIC ? page : nullptr;
        }
    }
    return nullptr;
}

[[nodiscard]] inline size_t large_table_index(void* ptr) noexcept {
    return (reinterpret_cast<uintptr_t>(ptr) >> 4) & (LARGE_TABLE_SIZE - 1);
}

void large_table_insert(void* ptr, LargeHeader* header) noexcept {
    pthread_mutex_lock(&large_table_lock);
    size_t idx = large_table_index(ptr);
    for (size_t n = 0; n < LARGE_TABLE_SIZE; ++n) {
        LargeTableEntry& e = large_table[(idx + n) & (LARGE_TABLE_SIZE - 1)];
        void* cur = e.ptr.load(std::memory_order_acquire);
        if (!cur || cur == ptr) {
            e.header.store(header, std::memory_order_release);
            e.ptr.store(ptr, std::memory_order_release);
            break;
        }
    }
    pthread_mutex_unlock(&large_table_lock);
}

[[nodiscard]] LargeHeader* large_table_lookup(void* ptr) noexcept {
    size_t idx = large_table_index(ptr);
    for (size_t n = 0; n < LARGE_TABLE_SIZE; ++n) {
        LargeTableEntry& e = large_table[(idx + n) & (LARGE_TABLE_SIZE - 1)];
        void* cur = e.ptr.load(std::memory_order_acquire);
        if (!cur) return nullptr;
        if (cur == ptr) {
            LargeHeader* h = e.header.load(std::memory_order_acquire);
            return h && h->magic == LARGE_MAGIC ? h : nullptr;
        }
    }
    return nullptr;
}

void large_table_remove(void* ptr) noexcept {
    pthread_mutex_lock(&large_table_lock);
    size_t idx = large_table_index(ptr);
    for (size_t n = 0; n < LARGE_TABLE_SIZE; ++n) {
        LargeTableEntry& e = large_table[(idx + n) & (LARGE_TABLE_SIZE - 1)];
        void* cur = e.ptr.load(std::memory_order_acquire);
        if (!cur) break;
        if (cur == ptr) {
            e.header.store(nullptr, std::memory_order_release);
            break;
        }
    }
    pthread_mutex_unlock(&large_table_lock);
}

void push_list(FreeObj*& list, size_t& count, FreeObj* head, FreeObj* tail, size_t n) noexcept {
    if (!head) return;
    tail->next = list;
    list = head;
    count += n;
}

[[nodiscard]] FreeObj* pop_list(FreeObj*& list, size_t& count) noexcept {
    FreeObj* obj = list;
    if (!obj) return nullptr;
    list = obj->next;
    count--;
    return obj;
}

void central_push(size_t idx, FreeObj* head, FreeObj* tail, size_t n) noexcept {
    CentralClass& c = tc_central[idx];
    pthread_mutex_lock(&c.lock);
    tail->next = c.list;
    c.list = head;
    c.count += n;
    pthread_mutex_unlock(&c.lock);
}

[[nodiscard]] FreeObj* central_take(size_t idx, size_t max, size_t& out_count) noexcept {
    out_count = 0;
    CentralClass& c = tc_central[idx];
    pthread_mutex_lock(&c.lock);
    FreeObj* head = c.list;
    FreeObj* cur = head;
    FreeObj* tail = nullptr;
    while (cur && out_count < max) {
        tail = cur;
        cur = cur->next;
        out_count++;
    }
    if (tail) {
        c.list = cur;
        tail->next = nullptr;
        c.count -= out_count;
    }
    pthread_mutex_unlock(&c.lock);
    return head;
}

[[nodiscard]] PageHeader* allocate_page(Family family, size_t idx, uint32_t owner_heap) noexcept {
    void* raw = ::mmap(nullptr, PAGE_SIZE * 2, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) return nullptr;

    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t aligned = (raw_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    size_t prefix = aligned - raw_addr;
    size_t suffix = (raw_addr + PAGE_SIZE * 2) - (aligned + PAGE_SIZE);
    if (prefix) ::munmap(reinterpret_cast<void*>(raw_addr), prefix);
    if (suffix) ::munmap(reinterpret_cast<void*>(aligned + PAGE_SIZE), suffix);

    auto* page = reinterpret_cast<PageHeader*>(aligned);
    page->magic = PAGE_MAGIC;
    page->family = family;
    page->class_idx = static_cast<uint16_t>(idx);
    page->object_size = static_cast<uint16_t>(class_size(idx));
    page->owner_heap = owner_heap;
    page->remote_free.store(nullptr, std::memory_order_relaxed);
    page->local_free = nullptr;
    page->local_count = 0;
    page->next_page = nullptr;

    uintptr_t start = aligned + sizeof(PageHeader);
    start = (start + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK;
    size_t usable = PAGE_SIZE - (start - aligned);
    page->object_count = static_cast<uint32_t>(usable / page->object_size);

    FreeObj* head = nullptr;
    FreeObj* tail = nullptr;
    for (size_t i = 0; i < page->object_count; ++i) {
        auto* obj = reinterpret_cast<FreeObj*>(start + i * page->object_size);
        obj->next = nullptr;
        if (!head) head = tail = obj;
        else {
            tail->next = obj;
            tail = obj;
        }
    }
    page->local_free = head;
    page->local_count = page->object_count;
    table_insert(reinterpret_cast<void*>(aligned), page);
    return page;
}

[[nodiscard]] void* large_alloc(size_t size, size_t alignment = MALLOC_ALIGNMENT) noexcept {
    if (alignment < MALLOC_ALIGNMENT) alignment = MALLOC_ALIGNMENT;
    size_t extra = alignment + sizeof(LargeHeader);
    size_t map_size = size + extra;
    void* raw = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) return nullptr;
    uintptr_t start = reinterpret_cast<uintptr_t>(raw) + sizeof(LargeHeader);
    uintptr_t user = (start + alignment - 1) & ~(alignment - 1);
    auto* h = reinterpret_cast<LargeHeader*>(user) - 1;
    h->magic = LARGE_MAGIC;
    h->mapping = raw;
    h->mapping_size = map_size;
    h->usable_size = size;
    void* ptr = reinterpret_cast<void*>(user);
    large_table_insert(ptr, h);
    return ptr;
}

void large_free(void* ptr, LargeHeader* h) noexcept {
    void* mapping = h->mapping;
    size_t mapping_size = h->mapping_size;
    h->magic = 0;
    large_table_remove(ptr);
    ::munmap(mapping, mapping_size);
}

void tc_drain(size_t idx) noexcept {
    FreeObj* head = tc_local[idx];
    FreeObj* tail = nullptr;
    size_t n = 0;
    while (head && n < DRAIN_BATCH) {
        tail = head;
        head = head->next;
        n++;
    }
    if (!tail) return;
    FreeObj* drain_head = tc_local[idx];
    tc_local[idx] = head;
    tc_count[idx] -= n;
    tail->next = nullptr;
    central_push(idx, drain_head, tail, n);
}

void je_tcache_drain(size_t idx) noexcept {
    FreeObj* head = je_tcache[idx];
    FreeObj* tail = nullptr;
    size_t n = 0;
    while (head && n < DRAIN_BATCH) {
        tail = head;
        head = head->next;
        n++;
    }
    if (!tail) return;
    FreeObj* drain_head = je_tcache[idx];
    je_tcache[idx] = head;
    je_tcache_count[idx] -= n;
    tail->next = nullptr;

    size_t arena_idx = je_arena_index == SIZE_MAX ? 0 : je_arena_index;
    JemallocArenaClass& ac = je_arenas[arena_idx].classes[idx];
    pthread_mutex_lock(&ac.lock);
    while (drain_head) {
        FreeObj* obj = drain_head;
        drain_head = obj->next;
        PageHeader* page = page_lookup(obj);
        obj->next = page->local_free;
        page->local_free = obj;
        page->local_count++;
    }
    pthread_mutex_unlock(&ac.lock);
}

void ensure_mi_heap() noexcept {
    if (mi_heap_id == 0) {
        mi_heap_id = next_heap_id.fetch_add(1, std::memory_order_relaxed);
    }
}

void mi_drain_remote(size_t idx) noexcept {
    ensure_mi_heap();
    for (PageHeader* page = mi_owned_pages[idx]; page; page = page->next_page) {
        FreeObj* list = page->remote_free.exchange(nullptr, std::memory_order_acquire);
        while (list) {
            FreeObj* obj = list;
            list = obj->next;
            obj->next = mi_local[idx];
            mi_local[idx] = obj;
            mi_count[idx]++;
        }
    }
}

} // namespace

void* tcmalloc_like_malloc(size_t size) noexcept {
    if (size == 0) size = 1;
    if (size > MAX_SMALL) return large_alloc(size);
    size_t idx = class_index(size);
    FreeObj* obj = pop_list(tc_local[idx], tc_count[idx]);
    if (!obj) {
        size_t n = 0;
        FreeObj* batch = central_take(idx, REFILL_BATCH, n);
        if (!batch) {
            PageHeader* page = allocate_page(Family::TcmallocLike, idx, 0);
            if (!page) return nullptr;
            FreeObj* tail = page->local_free;
            while (tail && tail->next) tail = tail->next;
            central_push(idx, page->local_free, tail, page->local_count);
            page->local_free = nullptr;
            page->local_count = 0;
            batch = central_take(idx, REFILL_BATCH, n);
        }
        FreeObj* tail = batch;
        for (size_t i = 1; tail && i < n; ++i) tail = tail->next;
        push_list(tc_local[idx], tc_count[idx], batch, tail, n);
        obj = pop_list(tc_local[idx], tc_count[idx]);
    }
    return obj;
}

void* jemalloc_like_malloc(size_t size) noexcept {
    if (size == 0) size = 1;
    if (size > MAX_SMALL) return large_alloc(size);
    if (je_arena_index == SIZE_MAX) {
        je_arena_index = next_arena.fetch_add(1, std::memory_order_relaxed) % JEMALLOC_ARENAS;
    }
    size_t idx = class_index(size);
    FreeObj* obj = pop_list(je_tcache[idx], je_tcache_count[idx]);
    if (obj) return obj;

    JemallocArenaClass& ac = je_arenas[je_arena_index].classes[idx];
    pthread_mutex_lock(&ac.lock);
    PageHeader* page = ac.pages;
    while (page && page->local_free == nullptr) page = page->next_page;
    if (!page) {
        page = allocate_page(Family::JemallocLike, idx, static_cast<uint32_t>(je_arena_index));
        if (!page) {
            pthread_mutex_unlock(&ac.lock);
            return nullptr;
        }
        page->next_page = ac.pages;
        ac.pages = page;
    }
    size_t moved = 0;
    while (page->local_free && moved < REFILL_BATCH) {
        FreeObj* cur = page->local_free;
        page->local_free = cur->next;
        page->local_count--;
        cur->next = je_tcache[idx];
        je_tcache[idx] = cur;
        je_tcache_count[idx]++;
        moved++;
    }
    pthread_mutex_unlock(&ac.lock);
    return pop_list(je_tcache[idx], je_tcache_count[idx]);
}

void* mimalloc_like_malloc(size_t size) noexcept {
    if (size == 0) size = 1;
    if (size > MAX_SMALL) return large_alloc(size);
    ensure_mi_heap();
    size_t idx = class_index(size);
    mi_drain_remote(idx);
    FreeObj* obj = pop_list(mi_local[idx], mi_count[idx]);
    if (obj) return obj;

    PageHeader* page = allocate_page(Family::MimallocLike, idx, mi_heap_id);
    if (!page) return nullptr;
    page->next_page = mi_owned_pages[idx];
    mi_owned_pages[idx] = page;
    FreeObj* head = page->local_free;
    FreeObj* tail = head;
    while (tail && tail->next) tail = tail->next;
    push_list(mi_local[idx], mi_count[idx], head, tail, page->local_count);
    page->local_free = nullptr;
    page->local_count = 0;
    return pop_list(mi_local[idx], mi_count[idx]);
}

bool family_free(void* ptr) noexcept {
    if (!ptr) return true;
    if (LargeHeader* h = large_table_lookup(ptr)) {
        large_free(ptr, h);
        return true;
    }
    PageHeader* page = page_lookup(ptr);
    if (!page) return false;
    size_t idx = page->class_idx;
    auto* obj = static_cast<FreeObj*>(ptr);
    switch (page->family) {
        case Family::TcmallocLike:
            obj->next = tc_local[idx];
            tc_local[idx] = obj;
            tc_count[idx]++;
            if (tc_count[idx] > LOCAL_LIMIT) tc_drain(idx);
            return true;
        case Family::JemallocLike:
            obj->next = je_tcache[idx];
            je_tcache[idx] = obj;
            je_tcache_count[idx]++;
            if (je_tcache_count[idx] > LOCAL_LIMIT) je_tcache_drain(idx);
            return true;
        case Family::MimallocLike:
            ensure_mi_heap();
            if (page->owner_heap == mi_heap_id) {
                obj->next = mi_local[idx];
                mi_local[idx] = obj;
                mi_count[idx]++;
            } else {
                FreeObj* old = page->remote_free.load(std::memory_order_relaxed);
                do {
                    obj->next = old;
                } while (!page->remote_free.compare_exchange_weak(
                    old, obj, std::memory_order_release, std::memory_order_relaxed));
            }
            return true;
    }
    return false;
}

void* family_realloc(void* ptr, size_t size) noexcept {
    if (!ptr) return tcmalloc_like_malloc(size);
    if (size == 0) {
        (void)family_free(ptr);
        return nullptr;
    }
    size_t old_size = family_usable_size(ptr);
    if (old_size == 0) return nullptr;
    if (size <= old_size) return ptr;
    Family family = Family::TcmallocLike;
    if (PageHeader* page = page_lookup(ptr)) family = page->family;
    void* next = nullptr;
    switch (family) {
        case Family::TcmallocLike: next = tcmalloc_like_malloc(size); break;
        case Family::JemallocLike: next = jemalloc_like_malloc(size); break;
        case Family::MimallocLike: next = mimalloc_like_malloc(size); break;
    }
    if (!next) return nullptr;
    std::memcpy(next, ptr, old_size < size ? old_size : size);
    (void)family_free(ptr);
    return next;
}

size_t family_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;
    if (LargeHeader* h = large_table_lookup(ptr)) return h->usable_size;
    PageHeader* page = page_lookup(ptr);
    return page ? page->object_size : 0;
}

void* family_memalign(size_t alignment, size_t size) noexcept {
    if (alignment <= MALLOC_ALIGNMENT) return tcmalloc_like_malloc(size);
    if (alignment & (alignment - 1)) return nullptr;
    return large_alloc(size, alignment);
}

} // namespace my_ptmalloc
