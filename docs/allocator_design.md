# System Architecture

This document explains how the allocator is organized internally. It focuses on data structures, ownership relationships, and control flow. Allocator theory is covered separately in [allocator_families.md](allocator_families.md).

## 1. Design Goal

`my_malloc` is a learning allocator with two goals:

1. Keep a readable ptmalloc-style implementation: chunks, boundary tags, bins, arenas, top chunk, and mmap.
2. Add a modern small-object frontend: size classes, slabs, thread-local caches, and central refill/drain.

The result is a hybrid design:

```text
malloc request
  |
  +-- small normal allocation, size <= 1024
  |      -> slab allocator
  |
  +-- aligned / medium / large / fallback allocation
         -> ptmalloc-style allocator
```

Runtime mode:

| Mode | Meaning |
|---|---|
| `MY_MALLOC_MODE=hybrid` | Use slab frontend first, then ptmalloc-style fallback. This is the default. |
| `MY_MALLOC_MODE=ptmalloc` | Disable the slab frontend and use only the chunk/bin/arena path. |

## 2. Top-level Components

```mermaid
flowchart TB
    User["User program"]
    Hooks["C malloc hooks<br/>malloc/free/calloc/realloc/memalign"]
    API["my_malloc API"]
    Init["Allocator initialization"]

    Slab["Small-object slab allocator<br/><=1024B"]
    SlabLocal["Thread-local slab lists"]
    SlabCentral["Central per-class caches"]
    SlabLookup["64KB slab lookup table"]

    Ptmalloc["ptmalloc-style fallback"]
    Tcache["Thread-local tcache"]
    ArenaMgr["ArenaManager"]
    Arena["Arena"]
    Pipeline["Allocation pipeline"]
    Bins["BinManager"]
    Sys["SysMemory mmap/new heap"]

    User --> Hooks --> API --> Init
    API --> Slab
    Slab --> SlabLocal
    SlabLocal --> SlabCentral
    Slab --> SlabLookup
    API --> Ptmalloc
    Ptmalloc --> Tcache
    Ptmalloc --> ArenaMgr --> Arena
    Arena --> Pipeline --> Bins
    Arena --> Sys
```

Main source files:

| Component | Files |
|---|---|
| Public API | `include/my_ptmalloc/my_malloc.h`, `src/my_malloc.cpp` |
| LD_PRELOAD hooks | `include/my_ptmalloc/hooks.h`, `src/hooks.cpp` |
| Initialization | `src/init.cpp` |
| Slab allocator | `include/my_ptmalloc/slab_allocator.h`, `src/slab_allocator.cpp` |
| Allocation pipeline | `include/my_ptmalloc/alloc_pipeline.h`, `src/alloc_pipeline.cpp`, `src/malloc_impl.cpp` |
| Free / coalescing | `src/free_impl.cpp`, `src/consolidate.cpp`, `src/coalesce.cpp` |
| Realloc | `src/realloc_impl.cpp` |
| Arenas/heaps | `include/my_ptmalloc/arena.h`, `src/arena_manager.cpp`, `src/heap.cpp` |
| Bins | `include/my_ptmalloc/*bins*.h`, `src/large_bins.cpp`, `src/unsorted_bin.cpp` |
| Strategy lab | `include/my_ptmalloc/strategy.h`, `src/strategy.cpp`, `tools/bench_runner.cpp` |

## 3. Data Structure Relationship

```mermaid
classDiagram
    class ArenaManager {
      Arena main_arena_
      Arena* arena_list_
      Arena* free_list_
      atomic<int> narenas_
      get_arena(size)
      new_arena(size)
      reused_arena(avoid)
    }

    class Arena {
      pthread_mutex_t mutex_
      Chunk* top_
      Chunk* last_remainder_
      BinManager bins_
      size_t system_mem_
    }

    class BinManager {
      FastBins fastbins_
      SmallBins smallbins_
      LargeBins largebins_
      UnsortedBin unsorted_
      BinMap binmap_
    }

    class TcachePerthread {
      uint16_t counts_[76]
      TcacheEntry* entries_[76]
      uint64_t random_key_
    }

    class Chunk {
      size_t prev_size
      size_t size
      Chunk* fd
      Chunk* bk
      Chunk* fd_nextsize
      Chunk* bk_nextsize
    }

    class SlabHeader {
      magic
      class_idx
      object_size
      object_count
      free_count
    }

    class SlabClassCache {
      thread_local list
      central list
      refill_batch()
      drain_batch()
    }

    class HeapInfo {
      Arena* ar_ptr
      HeapInfo* prev
      size_t size
    }

    ArenaManager --> Arena
    Arena --> BinManager
    BinManager --> FastBins
    BinManager --> SmallBins
    BinManager --> LargeBins
    BinManager --> UnsortedBin
    BinManager --> BinMap
    TcachePerthread --> Chunk
    FastBins --> Chunk
    SmallBins --> Chunk
    LargeBins --> Chunk
    UnsortedBin --> Chunk
    SlabClassCache --> SlabHeader
    HeapInfo --> Arena
```

Ownership rules:

- `TcachePerthread` is per-thread. Tcache allocation/free does not lock an arena.
- `Arena` owns chunk-backed free lists. Its mutex must be held when touching small, unsorted, large, top, and most consolidation state.
- `FastBins` use atomic stacks, but consolidation still belongs to the arena path.
- `HeapInfo` maps non-main heap regions back to the owning arena during free.
- Slab objects are not chunk-backed. Their owner is recovered from a 64KB-aligned slab base lookup.

## 4. Chunk-backed Allocation Path

Chunk-backed allocation follows a ptmalloc-like sequence:

```mermaid
flowchart TD
    Req["request size"]
    Norm["request2size<br/>alignment + header + minimum chunk"]
    TC{"tcache hit?"}
    Arena["select arena"]
    Lock["lock arena"]
    Fast{"fastbin hit?"}
    Small{"small bin hit?"}
    Unsorted{"unsorted scan hit?"}
    Large{"large bin best-fit hit?"}
    Top{"top chunk enough?"}
    Sys["mmap or grow heap"]
    Return["return user pointer"]

    Req --> Norm --> TC
    TC -- yes --> Return
    TC -- no --> Arena --> Lock --> Fast
    Fast -- yes --> Return
    Fast -- no --> Small
    Small -- yes --> Return
    Small -- no --> Unsorted
    Unsorted -- yes --> Return
    Unsorted -- no --> Large
    Large -- yes --> Return
    Large -- no --> Top
    Top -- yes --> Return
    Top -- no --> Sys --> Return
```

Important implementation detail: tcache is checked before arena locking. That keeps the most common chunk-backed hit path lock-free.

The pipeline is stored in a fixed `std::array<AllocStrategy*, 7>` instead of a `std::vector`. This is intentional: constructing a vector during allocator initialization can call `operator new`, re-enter `malloc`, and crash before global allocator state exists.

## 5. Slab Allocation Path

The slab path handles normal allocations up to 1024 bytes in hybrid mode.

```mermaid
flowchart TD
    Req["malloc(size <= 1024)"]
    Class["round up to 16-byte class"]
    Local{"thread-local list has object?"}
    Central{"central class cache has batch?"}
    NewSlab["allocate 64KB slab"]
    Publish["split slab into objects<br/>publish to central"]
    Pop["pop object"]
    Return["return object pointer"]

    Req --> Class --> Local
    Local -- yes --> Pop --> Return
    Local -- no --> Central
    Central -- yes --> Pop --> Return
    Central -- no --> NewSlab --> Publish --> Pop --> Return
```

Slab layout:

```text
64KB aligned address
|
+-- SlabHeader
|     magic
|     class index
|     object size
|     object count
|     free count
|
+-- object 0
+-- object 1
+-- object 2
+-- ...
```

Why this path exists:

- small objects avoid per-object chunk headers;
- fixed-size objects do not need boundary-tag coalescing;
- thread-local lists make the hot path fast;
- central caches reduce per-thread memory isolation by allowing batch exchange.

Current limitation: empty slabs are not yet returned to the OS. This is a major reason RSS can exceed glibc in fragmentation-heavy or phase-changing workloads.

## 6. Free Path

```mermaid
flowchart TD
    Free["free(ptr)"]
    Null{"ptr == null?"}
    SlabLookup["mask ptr to 64KB base<br/>lookup slab header"]
    IsSlab{"slab object?"}
    SlabFree["push to slab class list<br/>drain if too large"]
    Chunk["convert user ptr to Chunk"]
    Mmap{"IS_MMAPPED?"}
    Tcache{"tcache accepts?"}
    Arena["find owning arena"]
    Lock["lock arena"]
    Consolidate["coalesce forward/backward"]
    Top{"adjacent to top?"}
    Unsorted["insert into unsorted bin"]

    Free --> Null
    Null -- yes --> Done["return"]
    Null -- no --> SlabLookup --> IsSlab
    IsSlab -- yes --> SlabFree --> Done
    IsSlab -- no --> Chunk --> Mmap
    Mmap -- yes --> Munmap["munmap"] --> Done
    Mmap -- no --> Tcache
    Tcache -- yes --> Done
    Tcache -- no --> Arena --> Lock --> Consolidate --> Top
    Top -- yes --> MergeTop["absorb into top"] --> Done
    Top -- no --> Unsorted --> Done
```

The free path first distinguishes slab pointers from chunk pointers. This is necessary because slab objects do not have chunk headers.

## 7. Realloc Path

`realloc` tries to avoid copying:

1. `realloc(nullptr, size)` becomes `malloc(size)`.
2. `realloc(ptr, 0)` frees `ptr` and returns null.
3. If the existing usable size is enough, keep the allocation.
4. For chunk-backed allocations, try to grow into adjacent free space or top chunk.
5. Otherwise allocate a new object, copy the old contents, and free the old object.

Slab realloc currently uses the generic path when it crosses size classes. This keeps the implementation simple, but it is not as optimized as production allocators that can sometimes move between size classes with tighter policies.

## 8. LD_PRELOAD Bootstrap

Interposing `malloc` is tricky because dynamic loader and libc initialization can allocate memory before this allocator is fully ready.

This project uses:

- a small static bootstrap buffer before real symbols are resolved;
- real libc allocation for some early hook initialization cases;
- tracking of real-bootstrap pointers so later `free`/`realloc` calls go back to libc instead of this allocator.

This prevents a class of early startup crashes where memory allocated by libc was accidentally freed through `my_malloc`.

## 9. Observability

Observability is opt-in:

```text
MY_MALLOC_STATS=1
MY_MALLOC_TRACE=1
```

The default hot path avoids always-on counters because atomic telemetry can change benchmark results. This is an important allocator engineering lesson: measurement code can become part of the workload.

## 10. Main Simplifications

| Area | Simplification | Consequence |
|---|---|---|
| Slab reclamation | Empty slabs are cached, not unmapped | Higher RSS in fragmentation and phase-changing workloads |
| Remote free | Slab frees return to the current thread path | Producer/consumer workloads can be worse than mimalloc/tcmalloc |
| Size classes | Uniform 16-byte classes up to 1024B | Easier to understand, less tuned than production tables |
| Large bins | Sorted list plus binmap, not full glibc nextsize behavior | Simpler maintenance, less optimal search/update behavior |
| Adaptive policy | Coarse runtime modes | Good for experiments, not yet a fine-grained online selector |
| System memory | Simplified heap/span management | Less release/reuse sophistication than jemalloc/tcmalloc/mimalloc |
