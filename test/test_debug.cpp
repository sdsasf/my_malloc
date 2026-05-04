// Debug test: PURE alloc/free, NO memset, random sizes
#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/chunk.h"
#include "my_ptmalloc/config.h"
#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <random>
#include <atomic>

using namespace my_ptmalloc;

static std::atomic<int> done{0};
static std::atomic<int> corruption_count{0};

static void* worker(void* arg) {
    int id = *(int*)arg;
    std::mt19937 rng(id * 1000 + 42);
    std::uniform_int_distribution<size_t> size_dist(1, 2048);

    constexpr int SLOTS = 32;
    void* ptrs[SLOTS] = {};

    for (int i = 0; i < 5000 && !done.load(); ++i) {
        int idx = rng() % SLOTS;

        if (ptrs[idx] == nullptr) {
            size_t sz = size_dist(rng);
            void* p = my_malloc(sz);
            if (!p) continue;

            Chunk* c = Chunk::from_user_ptr(p);
            size_t cs = c->chunk_size().value;

            if (cs < MINSIZE || (cs & MALLOC_ALIGN_MASK) != 0) {
                fprintf(stderr, "[T%d] CAUGHT: i=%d sz=%zu ptr=%p chunk=%p raw=0x%zx prev=0x%zx\n",
                        id, i, sz, p, (void*)c, c->size, c->prev_size);
                corruption_count++;
                done.store(1);
                my_free(p);
                break;
            }

            // NO memset — just allocate and store
            ptrs[idx] = p;
        } else {
            my_free(ptrs[idx]);
            ptrs[idx] = nullptr;
        }
    }

    for (int i = 0; i < SLOTS; ++i) {
        if (ptrs[i]) my_free(ptrs[i]);
    }
    return nullptr;
}

int main() {
    printf("=== PURE alloc/free (no memset) ===\n");

    constexpr int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        ids[i] = i;
        pthread_create(&threads[i], nullptr, worker, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    printf("Corruption: %d\n", corruption_count.load());
    return corruption_count.load() > 0 ? 1 : 0;
}
