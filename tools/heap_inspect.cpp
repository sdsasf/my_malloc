// Heap inspection tool: dump arena/bin/chunk state for debugging

#include "my_ptmalloc/my_malloc.h"
#include "my_ptmalloc/arena.h"
#include "my_ptmalloc/arena_manager.h"
#include "my_ptmalloc/config.h"
#include "my_ptmalloc/chunk.h"
#include <cstdio>

using namespace my_ptmalloc;

static void dump_arena(const Arena& av, const char* name) {
    printf("=== Arena: %s ===\n", name);
    printf("  flags: 0x%x\n", av.flags_);
    printf("  top: %p\n", (void*)av.top_);
    printf("  last_remainder: %p\n", (void*)av.last_remainder_);
    printf("  attached_threads: %zu\n", av.attached_threads_);
    printf("  system_mem: %zu\n", av.system_mem_);
    printf("  max_system_mem: %zu\n", av.max_system_mem_);

    if (av.top_) {
        printf("  top chunk:\n");
        printf("    addr: %p\n", (void*)av.top_);
        printf("    size: %zu (flags: 0x%zx)\n",
               av.top_->chunk_size().value,
               av.top_->size & SIZE_BITS);
    }
}

int main() {
    printf("=== my_ptmalloc heap inspector ===\n\n");

    // Initialize allocator
    my_malloc_init();

    // Do some allocations to populate the heap
    void* p1 = my_malloc(64);
    void* p2 = my_malloc(128);
    void* p3 = my_malloc(256);
    void* p4 = my_malloc(512);

    printf("Allocations:\n");
    printf("  p1 (64B):   %p\n", p1);
    printf("  p2 (128B):  %p\n", p2);
    printf("  p3 (256B):  %p\n", p3);
    printf("  p4 (512B):  %p\n", p4);

    // Dump main arena
    if (g_arena_manager) {
        dump_arena(*g_arena_manager->get_main_arena(), "main_arena");
    }

    // Free and inspect
    printf("\nFreeing p2...\n");
    my_free(p2);

    printf("Freeing p4...\n");
    my_free(p4);

    // Allocate again to show reuse
    void* p5 = my_malloc(100);
    printf("\nAllocated p5 (100B): %p\n", p5);

    // Cleanup
    my_free(p1);
    my_free(p3);
    my_free(p5);

    printf("\nDone.\n");
    return 0;
}
