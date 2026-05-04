// Core realloc implementation

#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/chunk.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/types.h"
#include <cstring>

namespace my_ptmalloc {

void* my_realloc(void* ptr, size_t size) noexcept {
    // realloc(NULL, size) == malloc(size)
    if (!ptr) return my_malloc(size);

    // realloc(ptr, 0) == free(ptr), return NULL
    if (size == 0) {
        my_free(ptr);
        return nullptr;
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
