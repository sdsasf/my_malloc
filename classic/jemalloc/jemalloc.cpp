// jemalloc core implementation.
// Multi-arena → bin → run hierarchy with per-thread tcache and extent management.

#include "jemalloc.h"
#include "extent.h"
#include "rtree.h"

#include <pthread.h>
#include <sys/mman.h>
#include <atomic>
#include <cstring>
#include <new>
#include <cstdlib>

namespace my_ptmalloc {
namespace jemalloc {

// ─── Constants ───
constexpr unsigned JE_NARENAS     = 8;       // number of arenas
constexpr unsigned JE_TCACHE_MAX  = 4096;    // max tcache entries per size class
constexpr unsigned JE_TCACHE_GC_THRESH = 4096;  // tcache GC threshold (entries)
constexpr size_t   JE_RUN_MAX_PAGES  = 32;   // max pages per run

// ─── Run bitmap ───
// One bit per slot. Byte array, bit operations.
struct RunBitmap {
    unsigned char* bits;
    size_t         nslots;

    void init(size_t n) noexcept {
        nslots = n;
        size_t bytes = (n + 7) / 8;
        bits = new unsigned char[bytes]();
    }
    bool is_free(size_t idx) const noexcept {
        return !(bits[idx >> 3] & (1u << (idx & 7)));
    }
    void set_used(size_t idx) noexcept {
        bits[idx >> 3] |= (1u << (idx & 7));
    }
    void set_free(size_t idx) noexcept {
        bits[idx >> 3] &= ~(1u << (idx & 7));
    }
    size_t count_free() const noexcept {
        size_t n = 0;
        for (size_t i = 0; i < nslots; i++) if (is_free(i)) n++;
        return n;
    }
};

// ─── Bin (per-size-class, per-arena) ───
struct je_bin_t {
    pthread_mutex_t lock;
    extent_t*       runcur;    // current non-full run
    extent_t*       runs;      // all runs in this bin (linked via extent.prev/next)
    size_t          obj_size;  // size of objects in this bin
    size_t          nslots;    // objects per run
};

// ─── Arena ───
struct je_arena_t {
    unsigned       id;
    pthread_mutex_t lock;
    je_bin_t       bins[JE_NSMALL_CLASSES];
    extent_t*      extents_dirty;    // dirty extent tree
    extent_t*      extents_muzzy;
    extent_t*      extents_clean;
    size_t         allocated;
    size_t         resident;
};

// ─── Tcache (per-thread, per-size-class LIFO stack) ───
struct je_tcache_bin_t {
    void** stack;     // array of pointers
    size_t count;     // current entries
    size_t low_water; // refill threshold
};

struct je_tcache_t {
    je_tcache_bin_t bins[JE_NSMALL_CLASSES];
    unsigned        arena_idx;  // assigned arena
    size_t          total_count;

    void init(unsigned arena) noexcept;
    void* alloc(size_t class_idx) noexcept;
    bool  free(size_t class_idx, void* ptr) noexcept;
    void  flush(size_t class_idx, je_bin_t* bin) noexcept;
    void  gc() noexcept;
};

// ─── Globals ───
static je_arena_t       je_arenas[JE_NARENAS];
static std::atomic<unsigned> je_next_arena{0};
static bool             je_initialized = false;
static pthread_mutex_t  je_init_lock = PTHREAD_MUTEX_INITIALIZER;

thread_local je_tcache_t* je_tcache = nullptr;
thread_local unsigned      je_tcache_arena = 0;

// ─── rtree ───
rtree_t g_rtree;

rtree_t::rtree_t() noexcept {
    for (size_t i = 0; i < RTREE_NSLOTS; i++) {
        slots[i].store(nullptr, std::memory_order_release);
    }
}

extent_t* rtree_t::get(void* addr) const noexcept {
    size_t idx = (reinterpret_cast<uintptr_t>(addr) >> RTREE_SHIFT) % RTREE_NSLOTS;
    return slots[idx].load(std::memory_order_acquire);
}

void rtree_t::set(void* addr, extent_t* ext) noexcept {
    size_t idx = (reinterpret_cast<uintptr_t>(addr) >> RTREE_SHIFT) % RTREE_NSLOTS;
    slots[idx].store(ext, std::memory_order_release);
}

void rtree_t::clear(void* addr) noexcept {
    size_t idx = (reinterpret_cast<uintptr_t>(addr) >> RTREE_SHIFT) % RTREE_NSLOTS;
    slots[idx].store(nullptr, std::memory_order_release);
}

// ─── Extent management ───

extent_t* extent_alloc(size_t pages) noexcept {
    size_t bytes = pages * JE_PAGE_SIZE;
    bytes = (bytes + JE_PAGE_SIZE - 1) & ~JE_PAGE_MASK;
    void* addr = ::mmap(nullptr, bytes, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) return nullptr;

    auto* ext = new extent_t();
    ext->addr  = addr;
    ext->size  = bytes;
    ext->usize = bytes;
    ext->state = ExtentState::CLEAN;
    ext->arena_idx = 0;
    ext->nfree  = 0;
    ext->prev = ext->next = nullptr;
    ext->buddy_prev = ext->buddy_next = nullptr;
    ext->run_bitmap = nullptr;
    ext->run_nslots = 0;
    ext->run_obj_size = 0;

    // Map in rtree
    for (size_t i = 0; i < pages; i++) {
        g_rtree.set(static_cast<char*>(addr) + i * JE_PAGE_SIZE, ext);
    }
    return ext;
}

void extent_free(extent_t* ext) noexcept {
    // Clear rtree entries
    for (size_t i = 0; i < ext->size / JE_PAGE_SIZE; i++) {
        g_rtree.clear(static_cast<char*>(ext->addr) + i * JE_PAGE_SIZE);
    }
    if (ext->run_bitmap) delete[] static_cast<unsigned char*>(ext->run_bitmap);
    ::munmap(ext->addr, ext->size);
    delete ext;
}

// ─── Arena initialization ───

static void arena_init(je_arena_t* arena, unsigned id) noexcept {
    arena->id = id;
    pthread_mutex_init(&arena->lock, nullptr);
    arena->extents_dirty = nullptr;
    arena->extents_muzzy = nullptr;
    arena->extents_clean = nullptr;
    arena->allocated = 0;
    arena->resident  = 0;

    for (unsigned i = 0; i < JE_NSMALL_CLASSES; i++) {
        auto* bin = &arena->bins[i];
        pthread_mutex_init(&bin->lock, nullptr);
        bin->runcur   = nullptr;
        bin->runs     = nullptr;
        bin->obj_size = je_class_info[i].size;
        bin->nslots   = 0;
    }
}

// ─── Run allocation (allocates extent + initializes as run) ───

static extent_t* bin_alloc_run(je_bin_t* bin, unsigned arena_idx) noexcept {
    size_t obj_size = bin->obj_size;
    // Determine how many pages for a run
    // A run should hold enough objects to amortize overhead.
    // Target: at least 64 objects per run, max 32 pages.
    size_t target_pages = 1;
    size_t run_capacity  = JE_PAGE_SIZE / obj_size;
    while (run_capacity < 64 && target_pages < JE_RUN_MAX_PAGES) {
        target_pages++;
        run_capacity = (target_pages * JE_PAGE_SIZE) / obj_size;
    }

    extent_t* ext = extent_alloc(target_pages);
    if (!ext) return nullptr;

    bin->nslots = run_capacity;

    // Allocate bitmap
    ext->run_bitmap = new unsigned char[(run_capacity + 7) / 8]();
    ext->run_nslots   = run_capacity;
    ext->run_obj_size = obj_size;
    ext->nfree = static_cast<uint32_t>(run_capacity);
    ext->arena_idx = arena_idx;
    ext->state = ExtentState::DIRTY;

    // Link into bin's run list
    ext->next = bin->runs;
    if (bin->runs) bin->runs->prev = ext;
    bin->runs = ext;

    return ext;
}

// ─── Tcache ───

void je_tcache_t::init(unsigned arena) noexcept {
    arena_idx = arena;
    total_count = 0;
    for (unsigned i = 0; i < JE_NSMALL_CLASSES; i++) {
        bins[i].stack = static_cast<void**>(
            std::aligned_alloc(16, JE_TCACHE_MAX * sizeof(void*)));
        bins[i].count = 0;
        bins[i].low_water = 0;
    }
}

void* je_tcache_t::alloc(size_t class_idx) noexcept {
    if (class_idx >= JE_NSMALL_CLASSES) return nullptr;
    auto& bin = bins[class_idx];
    if (bin.count > bin.low_water) {
        void* p = bin.stack[--bin.count];
        total_count--;
        return p;
    }
    return nullptr;  // need refill
}

bool je_tcache_t::free(size_t class_idx, void* ptr) noexcept {
    if (class_idx >= JE_NSMALL_CLASSES) return false;
    auto& bin = bins[class_idx];
    if (bin.count < JE_TCACHE_MAX) {
        bin.stack[bin.count++] = ptr;
        total_count++;
        if (total_count > JE_TCACHE_GC_THRESH) gc();
        return true;
    }
    return false;  // full — flush to arena
}

void je_tcache_t::flush(size_t class_idx, je_bin_t* bin) noexcept {
    auto& tb = bins[class_idx];
    size_t n = tb.count;
    if (n == 0) return;

    extent_t* run = bin->runcur;
    if (!run) {
        // Need a run
        pthread_mutex_unlock(&bin->lock);
        run = bin_alloc_run(bin, arena_idx);
        if (!run) return;
        bin->runcur = run;
    }

    // Return objects to run's free slots
    for (size_t i = 0; i < n; i++) {
        void* obj = tb.stack[i];
        // Find slot index in run
        uintptr_t base = reinterpret_cast<uintptr_t>(run->addr);
        uintptr_t off  = reinterpret_cast<uintptr_t>(obj) - base;
        size_t slot = off / run->run_obj_size;
        if (slot < run->run_nslots) {
            // Mark as free
            auto* rb = static_cast<RunBitmap*>(run->run_bitmap);
            reinterpret_cast<RunBitmap*>(run->run_bitmap)->set_free(slot);
            run->nfree++;
        }
    }

    tb.count = 0;
    total_count -= n;

    // If run is now completely free, release it
    if (run->nfree == run->run_nslots) {
        bin->runcur = nullptr;
        // Unlink from bin
        if (run->prev) run->prev->next = run->next;
        else bin->runs = run->next;
        if (run->next) run->next->prev = run->prev;
        extent_free(run);
    }
}

void je_tcache_t::gc() noexcept {
    for (unsigned i = 0; i < JE_NSMALL_CLASSES; i++) {
        if (bins[i].count > bins[i].low_water * 2) {
            bins[i].count = bins[i].low_water;
            total_count -= (bins[i].count - bins[i].low_water);
        }
    }
}

// ─── Allocation ───

static void* je_large_alloc(size_t size) noexcept {
    size_t pages = (size + JE_PAGE_MASK) >> JE_LG_PAGE;
    extent_t* ext = extent_alloc(pages);
    if (!ext) return nullptr;
    ext->usize = size;
    ext->state = ExtentState::DIRTY;
    return ext->addr;
}

static void* je_huge_alloc(size_t size) noexcept {
    size_t map_size = (size + JE_PAGE_MASK) & ~JE_PAGE_MASK;
    void* addr = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (addr == MAP_FAILED) ? nullptr : addr;
}

void* je_malloc(size_t size) noexcept {
    je_init();
    if (size == 0) size = 1;

    // Huge: > 2MB → direct mmap
    if (size >= JE_HUGE_THRESHOLD) {
        return je_huge_alloc(size);
    }

    // Large: > small_max → page allocation
    if (size > JE_SMALL_MAX) {
        return je_large_alloc(size);
    }

    // Small: tcache → bin → run
    unsigned class_idx = je_size_to_class(size);
    je_tcache_t* tc = je_tcache;
    if (!tc) return nullptr;

    // Try tcache first
    void* p = tc->alloc(class_idx);
    if (p) return p;

    // Refill from arena bin
    unsigned arena_idx = tc->arena_idx;
    je_arena_t* arena = &je_arenas[arena_idx];
    je_bin_t* bin = &arena->bins[class_idx];

    pthread_mutex_lock(&bin->lock);

    // Get or create runcur
    extent_t* run = bin->runcur;
    if (!run || run->nfree == 0) {
        // Find a non-full run
        run = bin->runs;
        while (run && run->nfree == 0) run = run->next;
        if (!run) {
            run = bin_alloc_run(bin, arena_idx);
            if (!run) { pthread_mutex_unlock(&bin->lock); return nullptr; }
        }
        bin->runcur = run;
    }

    // Find a free slot
    auto* rb = reinterpret_cast<RunBitmap*>(run->run_bitmap);
    for (size_t s = 0; s < run->run_nslots; s++) {
        if (rb->is_free(s)) {
            void* obj = static_cast<char*>(run->addr) + s * bin->obj_size;
            rb->set_used(s);
            run->nfree--;
            pthread_mutex_unlock(&bin->lock);
            return obj;
        }
    }

    pthread_mutex_unlock(&bin->lock);
    return nullptr;  // shouldn't reach here normally
}

// ─── Free ───

void je_free(void* ptr) noexcept {
    if (!ptr) return;

    // Check huge (direct mmap): not in rtree
    extent_t* ext = g_rtree.get(ptr);
    if (!ext) {
        // Huge allocation — free the nearest page-aligned region
        uintptr_t aligned = reinterpret_cast<uintptr_t>(ptr) & ~JE_PAGE_MASK;
        ::munmap(reinterpret_cast<void*>(aligned),
                 JE_HUGE_THRESHOLD);  // approximate
        return;
    }

    if (ext->run_nslots > 0) {
        // Small object in a run
        unsigned class_idx = je_size_to_class(ext->run_obj_size);

        // Try tcache first
        je_tcache_t* tc = je_tcache;
        if (tc && tc->free(class_idx, ptr)) return;

        // Return to bin
        je_arena_t* arena = &je_arenas[ext->arena_idx];
        je_bin_t* bin = &arena->bins[class_idx];

        pthread_mutex_lock(&bin->lock);

        uintptr_t base = reinterpret_cast<uintptr_t>(ext->addr);
        uintptr_t off  = reinterpret_cast<uintptr_t>(ptr) - base;
        size_t slot = off / ext->run_obj_size;
        if (slot < ext->run_nslots) {
            auto* rb = reinterpret_cast<RunBitmap*>(ext->run_bitmap);
            rb->set_free(slot);
            ext->nfree++;
        }

        // If run is now completely free, release it
        if (ext->nfree == ext->run_nslots) {
            if (bin->runcur == ext) bin->runcur = nullptr;
            if (ext->prev) ext->prev->next = ext->next;
            else bin->runs = ext->next;
            if (ext->next) ext->next->prev = ext->prev;

            pthread_mutex_unlock(&bin->lock);
            extent_free(ext);
            return;
        }

        bin->runcur = ext;
        pthread_mutex_unlock(&bin->lock);
    } else {
        // Large object
        extent_free(ext);
    }
}

// ─── Calloc / Realloc / Memalign ───

void* je_calloc(size_t n, size_t size) noexcept {
    size_t total = n * size;
    if (n != 0 && total / n != size) return nullptr;
    void* p = je_malloc(total);
    if (p) std::memset(p, 0, je_usable_size(p) < total ? je_usable_size(p) : total);
    return p;
}

void* je_realloc(void* ptr, size_t size) noexcept {
    if (!ptr) return je_malloc(size);
    if (size == 0) { je_free(ptr); return nullptr; }
    size_t old_size = je_usable_size(ptr);
    if (old_size >= size) return ptr;
    void* np = je_malloc(size);
    if (!np) return nullptr;
    std::memcpy(np, ptr, old_size < size ? old_size : size);
    je_free(ptr);
    return np;
}

void* je_memalign(size_t alignment, size_t size) noexcept {
    if (alignment <= 16) return je_malloc(size);
    if ((alignment & (alignment - 1)) != 0) return nullptr;
    return je_large_alloc(size + alignment);  // simplified: use large alloc
}

size_t je_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;
    extent_t* ext = g_rtree.get(ptr);
    if (!ext) return JE_HUGE_THRESHOLD;
    if (ext->run_obj_size > 0) return ext->run_obj_size;
    return ext->usize;
}

// ─── Init ───

static void je_init_once() noexcept {
    for (unsigned i = 0; i < JE_NARENAS; i++) {
        arena_init(&je_arenas[i], i);
    }
    je_initialized = true;
}

void je_init() noexcept {
    if (je_initialized) return;
    pthread_mutex_lock(&je_init_lock);
    if (!je_initialized) {
        je_init_once();
    }
    pthread_mutex_unlock(&je_init_lock);

    // Assign thread to arena (round-robin)
    if (!je_tcache) {
        unsigned aid = je_next_arena.fetch_add(1, std::memory_order_relaxed) % JE_NARENAS;
        je_tcache = new je_tcache_t();
        je_tcache->init(aid);
        je_tcache_arena = aid;
    }
}

} // namespace jemalloc
} // namespace my_ptmalloc
