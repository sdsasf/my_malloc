// Public API wrappers

#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/slab_allocator.h"
#include "my_ptmalloc/family_allocators.h"
#include "my_ptmalloc/runtime_allocator.h"
#include "my_ptmalloc/allocator_lab.h"
#include "my_ptmalloc/adaptive_allocator.h"
#include <cerrno>
#include <cstring>
#include <new>

namespace my_ptmalloc {

void* my_calloc(size_t n, size_t size) noexcept {
    if (allocator_stats_enabled_fast()) stats_record_calloc();
    // Check for overflow
    size_t total = n * size;
    if (n != 0 && total / n != size) return nullptr;

    void* p = my_malloc(total);
    if (p) {
        std::memset(p, 0, total);
    }
    return p;
}

void* my_memalign(size_t alignment, size_t size) noexcept {
    if (alignment == 0) return nullptr;
    // Alignment must be power of 2 and >= sizeof(void*)
    if (alignment < sizeof(void*)) alignment = sizeof(void*);
    if (alignment & (alignment - 1)) {
        // Round up to next power of 2
        alignment--;
        alignment |= alignment >> 1;
        alignment |= alignment >> 2;
        alignment |= alignment >> 4;
        alignment |= alignment >> 8;
        alignment |= alignment >> 16;
        alignment |= alignment >> 32;
        alignment++;
        if (alignment == 0) return nullptr;
    }

    // Ensure alignment >= MALLOC_ALIGNMENT so chunk headers are always aligned
    if (alignment < MALLOC_ALIGNMENT) alignment = MALLOC_ALIGNMENT;
    if (alignment <= MALLOC_ALIGNMENT) return my_malloc(size);

    if (runtime_mode_is_adaptive()) {
        return adaptive_memalign(alignment, size);
    }

    if (runtime_mode_uses_family_allocators()) {
        RuntimeAllocatorKind impl = runtime_select_allocator(size);
        if (impl == RuntimeAllocatorKind::TcmallocLike ||
            impl == RuntimeAllocatorKind::JemallocLike ||
            impl == RuntimeAllocatorKind::MimallocLike) {
            return family_memalign(alignment, size);
        }
    }

    size_t nb = request2size(UserSize{size}).value;
    ScopedSlabBypass bypass;

    // Over-allocate so we can shift forward to the aligned position.
    // We need room for: nb usable bytes + up to (alignment - MALLOC_ALIGNMENT)
    // bytes of shift to reach alignment + alignment extra for fallback to the
    // NEXT alignment boundary when the first one yields a too-small front piece.
    if (alignment > (static_cast<size_t>(-1) - nb - CHUNK_HDR_SZ) / 2) {
        return nullptr;
    }
    size_t alloc_size = nb + 2 * alignment + CHUNK_HDR_SZ;
    void* raw = my_malloc(alloc_size);
    if (!raw) return nullptr;

    Chunk* p = Chunk::from_user_ptr(raw);
    size_t psize = p->chunk_size().value;

    // If already aligned, just return
    if ((reinterpret_cast<uintptr_t>(raw) & (alignment - 1)) == 0) {
        return raw;
    }

    // Compute the aligned chunk start: aligned_user_data - CHUNK_HDR_SZ
    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t aligned_user = (raw_addr + alignment - 1) & ~(alignment - 1);
    Chunk* aligned_chunk = reinterpret_cast<Chunk*>(aligned_user - CHUNK_HDR_SZ);

    // The front part (raw chunk to aligned chunk) would become a free chunk.
    size_t front_size = reinterpret_cast<uintptr_t>(aligned_chunk)
                      - reinterpret_cast<uintptr_t>(p);

    // If front piece is too small to be a valid free chunk (< MINSIZE),
    // advance to the NEXT alignment boundary.  This always works because
    // front_size is a multiple of MALLOC_ALIGNMENT (both p and raw are
    // MALLOC_ALIGNMENT-aligned), so the next boundary adds exactly
    // `alignment` bytes, giving front_size >= alignment >= MINSIZE.
    // The over-allocation (nb + 2*alignment + CHUNK_HDR_SZ) guarantees
    // we have room.
    if (front_size < MINSIZE) {
        aligned_user += alignment;
        aligned_chunk = reinterpret_cast<Chunk*>(aligned_user - CHUNK_HDR_SZ);
        front_size = reinterpret_cast<uintptr_t>(aligned_chunk)
                   - reinterpret_cast<uintptr_t>(p);
    }

    size_t remainder = psize - front_size;

    // Set up aligned chunk: PREV_INUSE set because front is being freed via
    // my_free which needs next->prev_inuse()==true to avoid forward merge.
    // The mark_inuse() sets the bit on the chunk AFTER aligned_chunk.
    aligned_chunk->set_head(ChunkSize{remainder}, ChunkFlag::PREV_INUSE);
    aligned_chunk->mark_inuse();

    // Set up front remainder as a free chunk and release it via normal free
    p->set_head(ChunkSize{front_size}, ChunkFlag::PREV_INUSE);
    my_free(p->user_data());

    return reinterpret_cast<void*>(aligned_user);
}

int my_posix_memalign(void** memptr, size_t alignment, size_t size) noexcept {
    if (!memptr) return 22;  // EINVAL
    if (alignment < sizeof(void*)) return 22;
    if (alignment & (alignment - 1)) return 22;

    void* p = my_memalign(alignment, size);
    if (!p) return 12;  // ENOMEM

    *memptr = p;
    return 0;
}

void* my_aligned_alloc(size_t alignment, size_t size) noexcept {
    // aligned_alloc requires size to be a multiple of alignment
    if (alignment == 0) return nullptr;
    if (alignment & (alignment - 1)) return nullptr;
    if (size % alignment != 0) return nullptr;
    return my_memalign(alignment, size);
}

int my_mallopt(int param, int value) noexcept {
    // Simplified: accept common params
    if (!g_arena_manager) return 1;

    auto* thresh = dynamic_cast<StaticThreshold*>(g_arena_manager->threshold());
    if (!thresh) return 0;

    switch (param) {
        case -1:  // M_MMAP_THRESHOLD
            thresh->set_mmap_threshold(value);
            return 1;
        case -2:  // M_TRIM_THRESHOLD
            thresh->set_trim_threshold(value);
            return 1;
        default:
            return 0;
    }
}

size_t my_malloc_usable_size(void* ptr) noexcept {
    if (!ptr) return 0;

    if (adaptive_owns(ptr)) {
        return adaptive_usable_size(ptr);
    }

    if (runtime_mode_uses_family_allocators()) {
        size_t family_usable = family_usable_size(ptr);
        if (family_usable != 0) return family_usable;
    }
    size_t slab_usable = slab_usable_size(ptr);
    if (slab_usable != 0) return slab_usable;
    Chunk* p = Chunk::from_user_ptr(ptr);
    size_t cs = p->chunk_size().value;
    // Usable = chunk_size - SIZE_SZ (prev_size of next chunk is usable)
    return cs > SIZE_SZ ? cs - SIZE_SZ : 0;
}

} // namespace my_ptmalloc
