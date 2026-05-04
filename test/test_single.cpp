// Single-threaded test to check if corruption is MT-only
#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/chunk.h"
#include "my_ptmalloc/config.h"
#include <cstdio>
#include <cstring>
#include <random>

using namespace my_ptmalloc;

int main() {
    printf("=== Single-thread random alloc/free ===\n");

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(1, 2048);

    constexpr int SLOTS = 32;
    void* ptrs[SLOTS] = {};
    int corruption = 0;

    for (int i = 0; i < 50000; ++i) {
        int idx = rng() % SLOTS;

        if (ptrs[idx] == nullptr) {
            size_t sz = size_dist(rng);
            void* p = my_malloc(sz);
            if (!p) continue;

            Chunk* c = Chunk::from_user_ptr(p);
            size_t cs = c->chunk_size().value;

            if (cs < MINSIZE || (cs & MALLOC_ALIGN_MASK) != 0) {
                fprintf(stderr, "[ST] CAUGHT: i=%d sz=%zu ptr=%p chunk=%p raw=0x%zx prev=0x%zx\n",
                        i, sz, p, (void*)c, c->size, c->prev_size);
                corruption++;
                my_free(p);
                break;
            }

            ptrs[idx] = p;
        } else {
            my_free(ptrs[idx]);
            ptrs[idx] = nullptr;
        }
    }

    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) my_free(ptrs[i]);
    }

    printf("Corruption: %d\n", corruption);
    return corruption > 0 ? 1 : 0;
}
