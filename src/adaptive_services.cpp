// Shared Memory Management Layer. This file owns concrete size-class pages,
// span-backed storage, direct mappings, ownership lookup, and reclaim paths.

#include "my_ptmalloc/adaptive_services.h"
#include "my_ptmalloc/adaptive_mode.h"
#include "my_ptmalloc/adaptive_runtime.h"
#include "my_ptmalloc/adaptive_telemetry.h"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <sys/mman.h>

namespace my_ptmalloc {

struct AdaptivePage {
    AdaptiveStorageId storage;
    AdaptiveModeId mode_id;
    uint32_t config_version;
    uint16_t class_index;
    uint16_t _pad;
    size_t block_usable;
    size_t block_stride;
    size_t mapped_size;
    uint32_t capacity;
    uint32_t live_count;
    AdaptiveHeader* free_list;
    AdaptivePage* next;
};

namespace {

constexpr uint8_t ADAPTIVE_FLAG_MEMALIGN = 1u << 0;
constexpr uint8_t ADAPTIVE_FLAG_POOLED   = 1u << 1;
constexpr uint8_t ADAPTIVE_FLAG_REDZONE  = 1u << 2;
constexpr size_t ADAPTIVE_SMALL_MAX = 1024;
constexpr size_t ADAPTIVE_MEDIUM_MAX = 64 * 1024;
constexpr size_t ADAPTIVE_SMALL_CLASS_STEP = 16;
constexpr size_t ADAPTIVE_MEDIUM_CLASS_STEP = 1024;
constexpr size_t ADAPTIVE_SMALL_CLASS_COUNT = ADAPTIVE_SMALL_MAX / ADAPTIVE_SMALL_CLASS_STEP;
constexpr size_t ADAPTIVE_MEDIUM_CLASS_COUNT =
    (ADAPTIVE_MEDIUM_MAX - ADAPTIVE_SMALL_MAX) / ADAPTIVE_MEDIUM_CLASS_STEP;
constexpr size_t ADAPTIVE_TCACHE_SMALL_MAX = 64;
constexpr size_t ADAPTIVE_TCACHE_MEDIUM_MAX = 16;
constexpr size_t ADAPTIVE_REMOTE_QUEUE_BUCKETS = 256;
constexpr size_t ADAPTIVE_REMOTE_DRAIN_LIMIT = 64;
constexpr size_t ADAPTIVE_DEBUG_REDZONE_SIZE = 16;
constexpr uint64_t ADAPTIVE_DEBUG_CANARY = 0xA6D4'BEEF'51E7'CAFEull;
constexpr size_t ADAPTIVE_PAGE_TABLE_SIZE = 262144;
constexpr size_t ADAPTIVE_PAGE_TABLE_PROBE = 16;
constexpr uintptr_t ADAPTIVE_PAGE_TOMBSTONE = static_cast<uintptr_t>(-1);

struct TCacheBin {
    AdaptiveHeader* head = nullptr;
    uint16_t count = 0;
};

struct AdaptiveThreadCache {
    TCacheBin small[ADAPTIVE_SMALL_CLASS_COUNT];
    TCacheBin medium[ADAPTIVE_MEDIUM_CLASS_COUNT];
};

struct RemoteQueueBucket {
    std::mutex mutex;
    uint64_t owner_thread = 0;
    AdaptiveHeader* head = nullptr;
};

std::mutex g_registry_mutex;
AdaptiveHeader* g_registry_head = nullptr;
std::mutex g_small_mutexes[ADAPTIVE_SMALL_CLASS_COUNT];
std::mutex g_medium_mutexes[ADAPTIVE_MEDIUM_CLASS_COUNT];
AdaptivePage* g_small_pages[ADAPTIVE_SMALL_CLASS_COUNT]{};
AdaptivePage* g_medium_pages[ADAPTIVE_MEDIUM_CLASS_COUNT]{};
std::atomic<uintptr_t> g_page_table[ADAPTIVE_PAGE_TABLE_SIZE]{};
RemoteQueueBucket g_remote_queues[ADAPTIVE_REMOTE_QUEUE_BUCKETS];
thread_local AdaptiveThreadCache t_thread_cache;

static void* user_from_header(AdaptiveHeader* hdr) noexcept {
    return reinterpret_cast<char*>(hdr) + ADAPTIVE_HDR_OFFSET;
}

static size_t align_up(size_t value, size_t alignment) noexcept {
    return (value + alignment - 1) & ~(alignment - 1);
}

static bool round_usable(size_t size, size_t& usable_out) noexcept {
    size_t usable = size == 0 ? 1 : size;
    if (usable > static_cast<size_t>(-1) - 15) return false;
    usable_out = (usable + 15) & ~size_t(15);
    return true;
}

static uintptr_t page_key(uintptr_t addr) noexcept {
    return addr & ~uintptr_t(4095);
}

static size_t page_table_index(uintptr_t page) noexcept {
    return (page >> 12) & (ADAPTIVE_PAGE_TABLE_SIZE - 1);
}

static void register_region(void* base, size_t size) noexcept {
    if (!base || size == 0) return;
    uintptr_t begin = page_key(reinterpret_cast<uintptr_t>(base));
    uintptr_t end = page_key(reinterpret_cast<uintptr_t>(base) + size - 1);
    for (uintptr_t page = begin; page <= end; page += 4096) {
        size_t idx = page_table_index(page);
        for (size_t probe = 0; probe < ADAPTIVE_PAGE_TABLE_PROBE; ++probe) {
            std::atomic<uintptr_t>& slot = g_page_table[(idx + probe) & (ADAPTIVE_PAGE_TABLE_SIZE - 1)];
            uintptr_t expected = 0;
            if (slot.compare_exchange_strong(expected, page, std::memory_order_relaxed) ||
                expected == page) {
                break;
            }
            if (expected == ADAPTIVE_PAGE_TOMBSTONE) {
                expected = ADAPTIVE_PAGE_TOMBSTONE;
                if (slot.compare_exchange_strong(expected, page, std::memory_order_relaxed)) break;
            }
        }
        if (page > static_cast<uintptr_t>(-1) - 4096) break;
    }
}

static void unregister_region(void* base, size_t size) noexcept {
    if (!base || size == 0) return;
    uintptr_t begin = page_key(reinterpret_cast<uintptr_t>(base));
    uintptr_t end = page_key(reinterpret_cast<uintptr_t>(base) + size - 1);
    for (uintptr_t page = begin; page <= end; page += 4096) {
        size_t idx = page_table_index(page);
        for (size_t probe = 0; probe < ADAPTIVE_PAGE_TABLE_PROBE; ++probe) {
            std::atomic<uintptr_t>& slot = g_page_table[(idx + probe) & (ADAPTIVE_PAGE_TABLE_SIZE - 1)];
            uintptr_t value = slot.load(std::memory_order_relaxed);
            if (value == page) {
                slot.store(ADAPTIVE_PAGE_TOMBSTONE, std::memory_order_relaxed);
                break;
            }
            if (value == 0) break;
        }
        if (page > static_cast<uintptr_t>(-1) - 4096) break;
    }
}

static bool page_maybe_owned(void* ptr) noexcept {
    if (!ptr) return false;
    uintptr_t page = page_key(reinterpret_cast<uintptr_t>(ptr));
    size_t idx = page_table_index(page);
    for (size_t probe = 0; probe < ADAPTIVE_PAGE_TABLE_PROBE; ++probe) {
        uintptr_t value = g_page_table[(idx + probe) & (ADAPTIVE_PAGE_TABLE_SIZE - 1)]
            .load(std::memory_order_relaxed);
        if (value == page) return true;
        if (value == 0) return false;
    }
    return false;
}

static bool valid_storage_id(AdaptiveStorageId id) noexcept {
    return id == AdaptiveStorageId::SizeClass ||
           id == AdaptiveStorageId::Span ||
           id == AdaptiveStorageId::DirectMap;
}

static uint64_t debug_cookie_for(AdaptiveHeader* hdr) noexcept {
    return ADAPTIVE_DEBUG_CANARY ^
        static_cast<uint64_t>(reinterpret_cast<uintptr_t>(hdr)) ^
        static_cast<uint64_t>(hdr->requested * 0x9E37'79B1u);
}

static void write_debug_guards(AdaptiveHeader* hdr) noexcept {
    if (!hdr || !(hdr->flags & ADAPTIVE_FLAG_REDZONE)) return;
    hdr->debug_cookie = debug_cookie_for(hdr);
    unsigned char* tail = static_cast<unsigned char*>(user_from_header(hdr)) + hdr->usable;
    for (size_t i = 0; i < ADAPTIVE_DEBUG_REDZONE_SIZE; ++i) {
        tail[i] = static_cast<unsigned char>((ADAPTIVE_DEBUG_CANARY >> ((i % 8) * 8)) & 0xFFu);
    }
}

static bool validate_debug_guards(AdaptiveHeader* hdr) noexcept {
    if (!hdr || !(hdr->flags & ADAPTIVE_FLAG_REDZONE)) return true;
    if (hdr->debug_cookie != debug_cookie_for(hdr)) return false;
    const unsigned char* tail = static_cast<const unsigned char*>(user_from_header(hdr)) + hdr->usable;
    for (size_t i = 0; i < ADAPTIVE_DEBUG_REDZONE_SIZE; ++i) {
        unsigned char expected = static_cast<unsigned char>((ADAPTIVE_DEBUG_CANARY >> ((i % 8) * 8)) & 0xFFu);
        if (tail[i] != expected) return false;
    }
    return true;
}

static bool header_plausible(AdaptiveHeader* hdr, void* user) noexcept {
    if (!hdr || hdr->magic != ADAPTIVE_MAGIC) return false;
    if (!valid_storage_id(hdr->storage)) return false;
    if (!adaptive_valid_mode_id(hdr->mode_id)) return false;
    if (user_from_header(hdr) != user) return false;
    if (hdr->usable < hdr->requested) return false;
    if (hdr->flags & ADAPTIVE_FLAG_POOLED) {
        return hdr->owner_page != nullptr && hdr->region_base == hdr->owner_page;
    }
    return hdr->region_base != nullptr && hdr->mapped_size >= ADAPTIVE_HDR_OFFSET;
}

static AdaptiveHeader* header_fast(void* ptr) noexcept {
    if (!ptr || !page_maybe_owned(ptr)) return nullptr;
    uintptr_t user = reinterpret_cast<uintptr_t>(ptr);
    if (user < ADAPTIVE_HDR_OFFSET) return nullptr;
    AdaptiveHeader* hdr = reinterpret_cast<AdaptiveHeader*>(user - ADAPTIVE_HDR_OFFSET);
    return header_plausible(hdr, ptr) ? hdr : nullptr;
}

static void registry_insert(AdaptiveHeader* hdr) noexcept {
    std::lock_guard<std::mutex> lock(g_registry_mutex);
    hdr->registry_prev = nullptr;
    hdr->registry_next = g_registry_head;
    if (g_registry_head) g_registry_head->registry_prev = hdr;
    g_registry_head = hdr;
}

static void registry_remove(AdaptiveHeader* hdr) noexcept {
    std::lock_guard<std::mutex> lock(g_registry_mutex);
    if (hdr->registry_prev) {
        hdr->registry_prev->registry_next = hdr->registry_next;
    } else if (g_registry_head == hdr) {
        g_registry_head = hdr->registry_next;
    }
    if (hdr->registry_next) hdr->registry_next->registry_prev = hdr->registry_prev;
    hdr->registry_prev = nullptr;
    hdr->registry_next = nullptr;
}

static bool small_class(size_t size, size_t& class_index, size_t& usable) noexcept {
    size_t rounded = size == 0 ? 1 : size;
    if (rounded > ADAPTIVE_SMALL_MAX) return false;
    rounded = align_up(rounded, ADAPTIVE_SMALL_CLASS_STEP);
    if (rounded == 0 || rounded > ADAPTIVE_SMALL_MAX) return false;
    class_index = (rounded / ADAPTIVE_SMALL_CLASS_STEP) - 1;
    usable = rounded;
    return true;
}

static bool medium_class(size_t size, size_t& class_index, size_t& usable) noexcept {
    size_t rounded = size == 0 ? 1 : size;
    if (rounded > ADAPTIVE_MEDIUM_MAX) return false;
    if (rounded <= ADAPTIVE_SMALL_MAX) rounded = ADAPTIVE_SMALL_MAX + 1;
    rounded = align_up(rounded, ADAPTIVE_MEDIUM_CLASS_STEP);
    if (rounded <= ADAPTIVE_SMALL_MAX || rounded > ADAPTIVE_MEDIUM_MAX) return false;
    class_index = ((rounded - ADAPTIVE_SMALL_MAX) / ADAPTIVE_MEDIUM_CLASS_STEP) - 1;
    usable = rounded;
    return class_index < ADAPTIVE_MEDIUM_CLASS_COUNT;
}

static AdaptivePage** pool_head_for(AdaptiveStorageId storage, size_t class_index) noexcept {
    if (storage == AdaptiveStorageId::SizeClass) {
        return class_index < ADAPTIVE_SMALL_CLASS_COUNT ? &g_small_pages[class_index] : nullptr;
    }
    if (storage == AdaptiveStorageId::Span) {
        return class_index < ADAPTIVE_MEDIUM_CLASS_COUNT ? &g_medium_pages[class_index] : nullptr;
    }
    return nullptr;
}

static std::mutex* pool_mutex_for(AdaptiveStorageId storage, size_t class_index) noexcept {
    if (storage == AdaptiveStorageId::SizeClass) {
        return class_index < ADAPTIVE_SMALL_CLASS_COUNT ? &g_small_mutexes[class_index] : nullptr;
    }
    if (storage == AdaptiveStorageId::Span) {
        return class_index < ADAPTIVE_MEDIUM_CLASS_COUNT ? &g_medium_mutexes[class_index] : nullptr;
    }
    return nullptr;
}

static TCacheBin* tcache_bin_for(AdaptiveStorageId storage, size_t class_index) noexcept {
    if (storage == AdaptiveStorageId::SizeClass) {
        return class_index < ADAPTIVE_SMALL_CLASS_COUNT ? &t_thread_cache.small[class_index] : nullptr;
    }
    if (storage == AdaptiveStorageId::Span) {
        return class_index < ADAPTIVE_MEDIUM_CLASS_COUNT ? &t_thread_cache.medium[class_index] : nullptr;
    }
    return nullptr;
}

static uint16_t tcache_limit_for(AdaptiveStorageId storage, AdaptiveModeId mode) noexcept {
    if (mode == AdaptiveModeId::DeterministicLatency) {
        if (storage == AdaptiveStorageId::SizeClass) return 8;
        if (storage == AdaptiveStorageId::Span) return 4;
        return 0;
    }
    if (mode == AdaptiveModeId::CrossThreadMessage) {
        if (storage == AdaptiveStorageId::SizeClass) return 32;
        if (storage == AdaptiveStorageId::Span) return 8;
        return 0;
    }
    if (mode == AdaptiveModeId::ThroughputCache) {
        if (storage == AdaptiveStorageId::SizeClass) return ADAPTIVE_TCACHE_SMALL_MAX;
        if (storage == AdaptiveStorageId::Span) return ADAPTIVE_TCACHE_MEDIUM_MAX;
        return 0;
    }
    return 0;
}

static size_t remote_bucket_index(uint64_t owner_thread) noexcept {
    return (owner_thread ^ (owner_thread >> 17) ^ (owner_thread >> 33)) %
        ADAPTIVE_REMOTE_QUEUE_BUCKETS;
}

static bool push_tcache(AdaptiveHeader* hdr) noexcept {
    if (!hdr || !(hdr->flags & ADAPTIVE_FLAG_POOLED) || !hdr->owner_page) return false;
    AdaptiveStorageId storage = hdr->owner_page->storage;
    size_t class_index = hdr->owner_page->class_index;
    TCacheBin* bin = tcache_bin_for(storage, class_index);
    if (!bin || bin->count >= tcache_limit_for(storage, hdr->mode_id)) return false;
    hdr->magic = 0;
    hdr->requested = 0;
    hdr->owner_thread = 0;
    hdr->registry_prev = nullptr;
    hdr->registry_next = bin->head;
    bin->head = hdr;
    bin->count++;
    return true;
}

static AdaptiveHeader* pop_tcache(AdaptiveStorageId storage, size_t class_index) noexcept {
    TCacheBin* bin = tcache_bin_for(storage, class_index);
    if (!bin || !bin->head) return nullptr;
    AdaptiveHeader* hdr = bin->head;
    bin->head = hdr->registry_next;
    bin->count--;
    hdr->registry_next = nullptr;
    hdr->registry_prev = nullptr;
    return hdr;
}

static bool enqueue_remote_free(AdaptiveHeader* hdr) noexcept {
    if (!hdr || !(hdr->flags & ADAPTIVE_FLAG_POOLED)) return false;
    size_t start = remote_bucket_index(hdr->owner_thread);
    for (size_t probe = 0; probe < ADAPTIVE_REMOTE_QUEUE_BUCKETS; ++probe) {
        RemoteQueueBucket& bucket = g_remote_queues[(start + probe) % ADAPTIVE_REMOTE_QUEUE_BUCKETS];
        std::lock_guard<std::mutex> lock(bucket.mutex);
        if (bucket.owner_thread == 0 || bucket.owner_thread == hdr->owner_thread) {
            bucket.owner_thread = hdr->owner_thread;
            hdr->magic = 0;
            hdr->requested = 0;
            hdr->registry_prev = nullptr;
            hdr->registry_next = bucket.head;
            bucket.head = hdr;
            return true;
        }
    }
    return false;
}

static void drain_remote_frees_for(uint64_t owner_thread) noexcept {
    if (owner_thread == 0) return;
    size_t start = remote_bucket_index(owner_thread);
    for (size_t probe = 0; probe < ADAPTIVE_REMOTE_QUEUE_BUCKETS; ++probe) {
        RemoteQueueBucket& bucket = g_remote_queues[(start + probe) % ADAPTIVE_REMOTE_QUEUE_BUCKETS];
        AdaptiveHeader* list = nullptr;
        {
            std::lock_guard<std::mutex> lock(bucket.mutex);
            if (bucket.owner_thread != owner_thread || !bucket.head) continue;
            list = bucket.head;
            bucket.head = nullptr;
        }
        size_t drained = 0;
        while (list && drained < ADAPTIVE_REMOTE_DRAIN_LIMIT) {
            AdaptiveHeader* next = list->registry_next;
            AdaptiveHeader* current = list;
            current->registry_next = nullptr;
            if (!push_tcache(current)) {
                current->registry_next = next;
                break;
            }
            list = next;
            drained++;
        }
        if (list) {
            std::lock_guard<std::mutex> lock(bucket.mutex);
            AdaptiveHeader* tail = list;
            while (tail->registry_next) tail = tail->registry_next;
            tail->registry_next = bucket.head;
            bucket.head = list;
        }
        return;
    }
}

static void finalize_pooled_header(AdaptiveHeader* hdr,
                                   const AllocationRequest& req,
                                   AdaptiveStorageId storage,
                                   size_t usable,
                                   size_t mapped_size,
                                   AdaptivePage* page) noexcept {
    hdr->magic = ADAPTIVE_MAGIC;
    hdr->storage = storage;
    hdr->mode_id = req.mode;
    hdr->flags = ADAPTIVE_FLAG_POOLED;
    hdr->_pad = 0;
    hdr->config_version = page->config_version;
    hdr->requested = req.size;
    hdr->usable = usable;
    hdr->mapped_size = mapped_size;
    hdr->region_base = page;
    hdr->registry_prev = nullptr;
    hdr->registry_next = nullptr;
    hdr->owner_page = page;
    hdr->owner_thread = req.thread_id;
    hdr->alloc_epoch = adaptive_next_alloc_epoch();
    hdr->debug_cookie = 0;
}

static AdaptivePage* create_pool_page(const AllocationRequest& req,
                                      AdaptiveStorageId storage,
                                      size_t class_index,
                                      size_t usable,
                                      size_t mapped_size) noexcept {
    size_t stride = align_up(ADAPTIVE_HDR_OFFSET + usable, 16);
    size_t first_block = align_up(sizeof(AdaptivePage), 16);
    if (stride == 0 || first_block >= mapped_size) return nullptr;
    size_t capacity = (mapped_size - first_block) / stride;
    if (capacity == 0 || capacity > static_cast<size_t>(UINT32_MAX)) return nullptr;

    void* region = mmap(nullptr, mapped_size, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) return nullptr;
    register_region(region, mapped_size);
    adaptive_telemetry_on_mapped(req.mode, mapped_size);

    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();
    AdaptivePage* page = static_cast<AdaptivePage*>(region);
    page->storage = storage;
    page->mode_id = req.mode;
    page->config_version = cfg.version;
    page->class_index = static_cast<uint16_t>(class_index);
    page->_pad = 0;
    page->block_usable = usable;
    page->block_stride = stride;
    page->mapped_size = mapped_size;
    page->capacity = static_cast<uint32_t>(capacity);
    page->live_count = 0;
    page->free_list = nullptr;
    page->next = nullptr;

    char* cursor = static_cast<char*>(region) + first_block;
    for (size_t i = 0; i < capacity; ++i) {
        AdaptiveHeader* hdr = reinterpret_cast<AdaptiveHeader*>(cursor + i * stride);
        hdr->magic = 0;
        hdr->storage = storage;
        hdr->mode_id = req.mode;
        hdr->flags = ADAPTIVE_FLAG_POOLED;
        hdr->_pad = 0;
        hdr->config_version = cfg.version;
        hdr->requested = 0;
        hdr->usable = usable;
        hdr->mapped_size = mapped_size;
        hdr->region_base = region;
        hdr->registry_prev = nullptr;
        hdr->registry_next = page->free_list;
        hdr->owner_page = page;
        hdr->owner_thread = 0;
        hdr->alloc_epoch = 0;
        hdr->debug_cookie = 0;
        page->free_list = hdr;
    }
    return page;
}

static uint32_t count_empty_locked(AdaptivePage* head) noexcept {
    uint32_t count = 0;
    for (AdaptivePage* page = head; page; page = page->next) {
        if (page->live_count == 0) count++;
    }
    return count;
}

static void release_page_locked(AdaptivePage** head, AdaptivePage* target, AdaptivePage* prev) noexcept {
    if (!head || !target || target->live_count != 0) return;
    if (prev) prev->next = target->next;
    else *head = target->next;
    size_t mapped = target->mapped_size;
    AdaptiveModeId mode = target->mode_id;
    AdaptiveStorageId storage = target->storage;
    adaptive_telemetry_on_release(storage, mapped);
    adaptive_telemetry_on_unmapped(mode, mapped);
    unregister_region(target, mapped);
    munmap(target, mapped);
    adaptive_telemetry_on_munmap();
}

static bool release_target_empty_locked(AdaptiveStorageId storage,
                                        AdaptivePage** head,
                                        AdaptivePage* target) noexcept {
    if (!head || !target || target->live_count != 0) return false;
    AdaptivePage* prev = nullptr;
    AdaptivePage* page = *head;
    while (page && page != target) {
        prev = page;
        page = page->next;
    }
    if (page != target) return false;
    release_page_locked(head, target, prev);
    adaptive_telemetry_on_empty_counts(storage, count_empty_locked(*head));
    return true;
}

static void release_empty_locked(AdaptiveStorageId storage, AdaptivePage** head, uint32_t keep) noexcept {
    if (!head) return;
    uint32_t empty = count_empty_locked(*head);
    adaptive_telemetry_on_empty_counts(storage, empty);
    while (empty > keep) {
        AdaptivePage* prev = nullptr;
        AdaptivePage* page = *head;
        while (page && page->live_count != 0) {
            prev = page;
            page = page->next;
        }
        if (!page) break;
        release_page_locked(head, page, prev);
        empty--;
    }
}

static AllocationResult fail_result() noexcept {
    return AllocationResult{};
}

static AdaptiveStorageId auto_storage_for(const AllocationRequest& req) noexcept {
    if (req.alignment > 16 || req.prefer_direct_map) return AdaptiveStorageId::DirectMap;
    if (req.size <= ADAPTIVE_SMALL_MAX) return AdaptiveStorageId::SizeClass;
    if (req.size <= ADAPTIVE_MEDIUM_MAX) return AdaptiveStorageId::Span;
    return AdaptiveStorageId::DirectMap;
}

} // namespace

AllocationResult MemoryServices::allocate_auto(const AllocationRequest& req) noexcept {
    switch (auto_storage_for(req)) {
        case AdaptiveStorageId::SizeClass: return allocate_size_class(req);
        case AdaptiveStorageId::Span:      return allocate_span(req);
        case AdaptiveStorageId::DirectMap: return allocate_direct_map(req);
    }
    return fail_result();
}

AllocationResult MemoryServices::allocate_size_class(const AllocationRequest& req) noexcept {
    if (req.alignment > 16 || req.size > ADAPTIVE_SMALL_MAX) return allocate_auto(req);
    size_t class_index = 0;
    size_t usable = 0;
    if (!small_class(req.size, class_index, usable)) return fail_result();
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();

    if (req.prefer_thread_cache) {
        drain_remote_frees_for(req.thread_id);
        if (AdaptiveHeader* cached = pop_tcache(AdaptiveStorageId::SizeClass, class_index)) {
            finalize_pooled_header(cached, req, AdaptiveStorageId::SizeClass, usable,
                                   cached->mapped_size, cached->owner_page);
            registry_insert(cached);
            return AllocationResult{user_from_header(cached), cached, AdaptiveStorageId::SizeClass,
                                    req.size, usable, cached->mapped_size, 0,
                                    true, false, false};
        }
    }

    AdaptiveHeader* hdr = nullptr;
    bool from_cache = true;
    bool slow_path = false;
    size_t newly_mapped = 0;
    AdaptivePage** head = pool_head_for(AdaptiveStorageId::SizeClass, class_index);
    std::mutex* mutex = pool_mutex_for(AdaptiveStorageId::SizeClass, class_index);
    if (!head || !mutex) return fail_result();
    {
        std::lock_guard<std::mutex> lock(*mutex);
        AdaptivePage* page = *head;
        while (page && !page->free_list) page = page->next;
        if (!page) {
            from_cache = false;
            slow_path = true;
            newly_mapped = cfg.small_page_size;
            page = create_pool_page(req, AdaptiveStorageId::SizeClass, class_index, usable, cfg.small_page_size);
            if (!page) return fail_result();
            page->next = *head;
            *head = page;
        }
        hdr = page->free_list;
        page->free_list = hdr->registry_next;
        page->live_count++;
        finalize_pooled_header(hdr, req, AdaptiveStorageId::SizeClass, usable,
                               page->mapped_size, page);
    }
    registry_insert(hdr);
    return AllocationResult{user_from_header(hdr), hdr, AdaptiveStorageId::SizeClass,
                            req.size, usable, hdr->mapped_size, newly_mapped,
                            from_cache, false, slow_path};
}

AllocationResult MemoryServices::allocate_span(const AllocationRequest& req) noexcept {
    if (req.alignment > 16 || req.size <= ADAPTIVE_SMALL_MAX || req.size > ADAPTIVE_MEDIUM_MAX) {
        return allocate_auto(req);
    }
    size_t class_index = 0;
    size_t usable = 0;
    if (!medium_class(req.size, class_index, usable)) return fail_result();
    AdaptiveRuntimeConfig cfg = adaptive_runtime_config();

    if (req.prefer_thread_cache) {
        drain_remote_frees_for(req.thread_id);
        if (AdaptiveHeader* cached = pop_tcache(AdaptiveStorageId::Span, class_index)) {
            finalize_pooled_header(cached, req, AdaptiveStorageId::Span, usable,
                                   cached->mapped_size, cached->owner_page);
            registry_insert(cached);
            return AllocationResult{user_from_header(cached), cached, AdaptiveStorageId::Span,
                                    req.size, usable, cached->mapped_size, 0,
                                    true, false, false};
        }
    }

    AdaptiveHeader* hdr = nullptr;
    bool from_cache = true;
    bool slow_path = false;
    size_t newly_mapped = 0;
    AdaptivePage** head = pool_head_for(AdaptiveStorageId::Span, class_index);
    std::mutex* mutex = pool_mutex_for(AdaptiveStorageId::Span, class_index);
    if (!head || !mutex) return fail_result();
    {
        std::lock_guard<std::mutex> lock(*mutex);
        AdaptivePage* page = *head;
        while (page && !page->free_list) page = page->next;
        if (!page) {
            from_cache = false;
            slow_path = true;
            newly_mapped = cfg.medium_span_size;
            page = create_pool_page(req, AdaptiveStorageId::Span, class_index, usable, cfg.medium_span_size);
            if (!page) return fail_result();
            page->next = *head;
            *head = page;
        }
        hdr = page->free_list;
        page->free_list = hdr->registry_next;
        page->live_count++;
        finalize_pooled_header(hdr, req, AdaptiveStorageId::Span, usable,
                               page->mapped_size, page);
    }
    registry_insert(hdr);
    return AllocationResult{user_from_header(hdr), hdr, AdaptiveStorageId::Span,
                            req.size, usable, hdr->mapped_size, newly_mapped,
                            from_cache, false, slow_path};
}

AllocationResult MemoryServices::allocate_extent(const AllocationRequest& req) noexcept {
    return allocate_direct_map(req);
}

AllocationResult MemoryServices::allocate_direct_map(const AllocationRequest& req) noexcept {
    size_t alignment = req.alignment < 16 ? 16 : req.alignment;
    size_t usable = 0;
    if (!round_usable(req.size, usable)) return fail_result();
    size_t redzone = req.debug_redzone ? ADAPTIVE_DEBUG_REDZONE_SIZE : 0;
    size_t align_slack = alignment > 16 ? alignment : 0;
    if (usable > static_cast<size_t>(-1) - ADAPTIVE_HDR_OFFSET) return fail_result();
    size_t total = ADAPTIVE_HDR_OFFSET + usable;
    if (redzone > static_cast<size_t>(-1) - total) return fail_result();
    total += redzone;
    if (align_slack > static_cast<size_t>(-1) - total) return fail_result();
    total += align_slack;
    constexpr size_t page_size = 4096;
    if (total > static_cast<size_t>(-1) - (page_size - 1)) return fail_result();
    size_t mapped = (total + page_size - 1) & ~(page_size - 1);
    void* region = mmap(nullptr, mapped, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) return fail_result();
    register_region(region, mapped);

    uintptr_t base_user = reinterpret_cast<uintptr_t>(region) + ADAPTIVE_HDR_OFFSET;
    uintptr_t aligned_user = alignment > 16 ? (base_user + alignment - 1) & ~(alignment - 1) : base_user;
    AdaptiveHeader* hdr = reinterpret_cast<AdaptiveHeader*>(aligned_user - ADAPTIVE_HDR_OFFSET);
    hdr->magic = ADAPTIVE_MAGIC;
    hdr->storage = AdaptiveStorageId::DirectMap;
    hdr->mode_id = req.mode;
    hdr->flags = (alignment > 16 ? ADAPTIVE_FLAG_MEMALIGN : 0) |
                 (req.debug_redzone ? ADAPTIVE_FLAG_REDZONE : 0);
    hdr->_pad = 0;
    hdr->config_version = adaptive_runtime_config().version;
    hdr->requested = req.size;
    hdr->usable = usable;
    hdr->mapped_size = mapped;
    hdr->region_base = region;
    hdr->registry_prev = nullptr;
    hdr->registry_next = nullptr;
    hdr->owner_page = nullptr;
    hdr->owner_thread = req.thread_id;
    hdr->alloc_epoch = adaptive_next_alloc_epoch();
    hdr->debug_cookie = 0;
    write_debug_guards(hdr);
    registry_insert(hdr);
    adaptive_telemetry_on_mapped(req.mode, mapped);
    return AllocationResult{reinterpret_cast<void*>(aligned_user), hdr, AdaptiveStorageId::DirectMap,
                            req.size, usable, mapped, mapped, false, true, true};
}

void MemoryServices::deallocate(AdaptiveHeader* hdr, const ReleaseDecision& decision) noexcept {
    if (!hdr) return;
    if (hdr->flags & ADAPTIVE_FLAG_POOLED) {
        AdaptivePage* page = hdr->owner_page;
        if (!page) return;
        AdaptiveStorageId storage = page->storage;
        size_t class_index = page->class_index;
        AdaptivePage** head = pool_head_for(storage, class_index);
        std::mutex* mutex = pool_mutex_for(storage, class_index);
        if (!head || !mutex) return;
        bool remote_free = hdr->owner_thread != adaptive_thread_token();
        bool use_remote_queue = hdr->mode_id == AdaptiveModeId::CrossThreadMessage && remote_free;
        bool use_tcache = decision.action == ReleaseAction::Cache && !remote_free;
        registry_remove(hdr);
        if (decision.check_redzone && !validate_debug_guards(hdr)) {
            adaptive_telemetry_on_header_corruption();
        }
        if (decision.poison && hdr->usable > 0) {
            std::memset(user_from_header(hdr), 0xDD, hdr->usable);
        }
        if (use_remote_queue && enqueue_remote_free(hdr)) {
            return;
        }
        if (use_tcache && push_tcache(hdr)) {
            return;
        }
        std::lock_guard<std::mutex> lock(*mutex);
        hdr->magic = 0;
        hdr->requested = 0;
        hdr->owner_thread = 0;
        hdr->registry_prev = nullptr;
        hdr->registry_next = page->free_list;
        page->free_list = hdr;
        if (page->live_count > 0) page->live_count--;
        if ((decision.action == ReleaseAction::Purge || decision.action == ReleaseAction::Unmap) &&
            page->live_count == 0 && release_target_empty_locked(storage, head, page)) {
            return;
        }
        uint32_t keep = decision.action == ReleaseAction::Purge ||
                        decision.action == ReleaseAction::Unmap ? 0 : adaptive_runtime_config().empty_cache_limit;
        if (decision.action == ReleaseAction::Cache) keep = adaptive_runtime_config().empty_cache_limit;
        release_empty_locked(storage, head, keep);
        return;
    }

    registry_remove(hdr);
    void* base = hdr->region_base;
    size_t mapped = hdr->mapped_size;
    AdaptiveModeId mode = hdr->mode_id;
    if (decision.check_redzone && !validate_debug_guards(hdr)) {
        adaptive_telemetry_on_header_corruption();
    }
    if (decision.poison && hdr->usable > 0) {
        std::memset(user_from_header(hdr), 0xDD, hdr->usable);
    }
    if (decision.quarantine || decision.action == ReleaseAction::Quarantine) {
        hdr->magic = 0;
        // HardenedDebug retains quarantined direct mappings so repeat frees
        // stay visible to adaptive diagnostics instead of another allocator.
        return;
    }
    adaptive_telemetry_on_unmapped(mode, mapped);
    unregister_region(base, mapped);
    munmap(base, mapped);
    adaptive_telemetry_on_munmap();
}

void* MemoryServices::reallocate(AdaptiveHeader* hdr, size_t new_size) noexcept {
    if (!hdr) return nullptr;
    void* old_user = user_from_header(hdr);
    if (new_size <= hdr->usable) return old_user;
    const AdaptiveModePolicy& policy = adaptive_mode_policy(hdr->mode_id);
    AllocationPlan plan = policy.plan_allocate(new_size, 16);
    AllocationRequest req{new_size, 16, hdr->mode_id, adaptive_thread_token(), false,
                          plan.use_thread_cache, plan.prefer_direct_map,
                          plan.prefer_reuse, plan.prefer_low_rss,
                          plan.debug_redzone, plan.debug_quarantine,
                          plan.batch_size, plan.empty_keep_limit};
    AllocationResult result{};
    switch (plan.storage) {
        case StoragePreference::SizeClass: result = allocate_size_class(req); break;
        case StoragePreference::Span:      result = allocate_span(req); break;
        case StoragePreference::Extent:    result = allocate_extent(req); break;
        case StoragePreference::DirectMap: result = allocate_direct_map(req); break;
        case StoragePreference::Auto:      result = allocate_auto(req); break;
    }
    if (!result.user_ptr) return nullptr;
    adaptive_telemetry_on_alloc(result);
    size_t copy = hdr->usable < new_size ? hdr->usable : new_size;
    std::memcpy(result.user_ptr, old_user, copy);
    adaptive_telemetry_on_free_begin(hdr);
    deallocate(hdr, policy.plan_free(hdr));
    adaptive_telemetry_on_free_end(hdr);
    return result.user_ptr;
}

size_t MemoryServices::usable_size(AdaptiveHeader* hdr) noexcept {
    return hdr ? hdr->usable : 0;
}

bool MemoryServices::owns(void* ptr) noexcept {
    return ptr && (header_from_user(ptr) != nullptr || page_maybe_owned(ptr));
}

AdaptiveHeader* MemoryServices::header_from_user(void* ptr) noexcept {
    if (AdaptiveHeader* hdr = header_fast(ptr)) return hdr;
    if (page_maybe_owned(ptr)) {
        uintptr_t user = reinterpret_cast<uintptr_t>(ptr);
        if (user >= ADAPTIVE_HDR_OFFSET) {
            AdaptiveHeader* candidate = reinterpret_cast<AdaptiveHeader*>(user - ADAPTIVE_HDR_OFFSET);
            if (page_maybe_owned(candidate) && candidate->magic == 0) {
                adaptive_telemetry_on_double_free();
            } else if (page_maybe_owned(candidate)) {
                adaptive_telemetry_on_header_corruption();
            }
        }
    }
    return nullptr;
}

MemoryServices& adaptive_memory_services() noexcept {
    static MemoryServices services;
    return services;
}

} // namespace my_ptmalloc
