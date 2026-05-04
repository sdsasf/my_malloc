// Core realloc implementation

#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/chunk.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/heap.h"
#include "my_ptmalloc/slab_allocator.h"
#include "my_ptmalloc/types.h"
#include <cstring>

namespace my_ptmalloc {

static bool chunk_is_free(Arena& arena, Chunk* p) noexcept {
    if (!p || p == arena.top()) return false;
    return arena.bins_.contains_free_chunk(p);
}

static void split_allocated_tail(Arena& arena, Chunk* p, ChunkSize nb,
                                 size_t total_size, ChunkFlag flags) noexcept {
    size_t remainder_size = total_size - nb.value;
    if (remainder_size >= MINSIZE) {
        p->set_head(nb, flags);
        Chunk* remainder = reinterpret_cast<Chunk*>(
            reinterpret_cast<uintptr_t>(p) + nb.value);
        remainder->prev_size = nb.value;
        remainder->set_head(ChunkSize{remainder_size}, flags | ChunkFlag::PREV_INUSE);
        remainder->set_foot(ChunkSize{remainder_size});
        remainder->clear_previnuse();
        arena.bins_.unsorted().push(remainder);
    } else {
        p->set_head(ChunkSize{total_size}, flags);
        p->mark_inuse();
    }
}

void* my_realloc(void* ptr, size_t size) noexcept {
    // realloc(NULL, size) == malloc(size)
    if (!ptr) return my_malloc(size);

    // realloc(ptr, 0) == free(ptr), return NULL
    if (size == 0) {
        my_free(ptr);
        return nullptr;
    }

    size_t slab_usable = slab_usable_size(ptr);
    if (slab_usable != 0) {
        if (size <= slab_usable) return ptr;
        void* new_ptr = my_malloc(size);
        if (!new_ptr) return nullptr;
        std::memcpy(new_ptr, ptr, slab_usable);
        my_free(ptr);
        return new_ptr;
    }

    Chunk* oldp = Chunk::from_user_ptr(ptr);
    size_t old_size = oldp->chunk_size().value;
    // Usable size: chunk_size - SIZE_SZ (the next chunk's prev_size overlaps
    // with the last SIZE_SZ bytes of this chunk's user data)
    size_t old_usable = old_size - SIZE_SZ;

    ChunkSize nb = request2size(UserSize{size});

    // If current chunk is big enough, return as-is
    if (old_size >= nb.value) {
        return ptr;
    }

    if (!oldp->is_mmapped()) {
        Arena* arena = HeapInfo::arena_for_chunk(oldp, oldp->is_main_arena());
        if (arena) {
            arena->lock();
            ChunkFlag flags = oldp->flags();
            Chunk* next = oldp->next_chunk();

            if (next == arena->top()) {
                size_t total = old_size + next->chunk_size().value;
                if (total >= nb.value) {
                    size_t remainder_size = total - nb.value;
                    if (remainder_size >= MINSIZE) {
                        Chunk* new_top = reinterpret_cast<Chunk*>(
                            reinterpret_cast<uintptr_t>(oldp) + nb.value);
                        new_top->prev_size = nb.value;
                        new_top->set_head(ChunkSize{remainder_size},
                                          flags | ChunkFlag::PREV_INUSE);
                        new_top->set_foot(ChunkSize{remainder_size});
                        arena->set_top(new_top);
                        oldp->set_head(nb, flags);
                    } else {
                        arena->set_top(nullptr);
                        oldp->set_head(ChunkSize{total}, flags);
                        oldp->mark_inuse();
                    }
                    arena->unlock();
                    return ptr;
                }
            }

            if (chunk_is_free(*arena, next)) {
                size_t next_size = next->chunk_size().value;
                size_t total = old_size + next_size;
                if (total >= nb.value && arena->bins_.unlink_free_chunk(next)) {
                    if (arena->last_remainder_ == next) arena->last_remainder_ = nullptr;
                    split_allocated_tail(*arena, oldp, nb, total, flags);
                    arena->unlock();
                    return ptr;
                }
            }

            arena->unlock();
        }
    }

    // Allocate new block
    void* new_ptr = my_malloc(size);
    if (!new_ptr) return nullptr;

    // Copy old data (min of old usable size and new size)
    size_t copy_size = old_usable < size ? old_usable : size;
    std::memcpy(new_ptr, ptr, copy_size);

    // Free old block
    my_free(ptr);

    return new_ptr;
}

} // namespace my_ptmalloc
