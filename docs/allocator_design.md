# my_ptmalloc Allocator Design

This document explains the current allocator architecture, the relationships between the major data structures, and the optimized allocation/free paths. It is written as a system design document rather than a line-by-line source tour.

## 1. Design Goals

`my_ptmalloc` is a compact C++17 reimplementation of the main ptmalloc ideas used by glibc malloc:

- chunk headers with boundary-tag metadata;
- thread-local tcache;
- fastbins, small bins, large bins, and an unsorted bin;
- arenas for reducing multi-threaded lock contention;
- top chunks for heap growth;
- direct mmap for large allocations;
- `HeapInfo` ownership lookup for non-main arenas.

The project deliberately differs from glibc in implementation style. Instead of one macro-heavy `malloc.c`, it uses small C++ classes: `ArenaManager`, `Arena`, `BinManager`, `TcachePerthread`, `FastBins`, `SmallBins`, `LargeBins`, and `UnsortedBin`.

## 2. System Overview

```mermaid
flowchart TB
    API["Public API<br/>my_malloc / my_free / my_realloc / hooks"]
    Init["Global Init<br/>ArenaManager + AllocPipeline"]
    TC["Thread-local Tcache<br/>76 bins, lock-free"]
    AM["ArenaManager<br/>arena creation / reuse / selection"]
    AR["Arena<br/>mutex + top + last_remainder + bins"]
    BM["BinManager"]
    FB["FastBins<br/>CAS LIFO stacks"]
    SB["SmallBins<br/>exact-size FIFO lists"]
    UB["UnsortedBin<br/>bounded staging scan"]
    LB["LargeBins<br/>sorted best-fit + binmap"]
    TOP["Top Chunk<br/>wilderness chunk"]
    SYS["SysMemory<br/>mmap / munmap"]
    HI["HeapInfo<br/>non-main arena owner lookup"]

    API --> Init
    API --> TC
    API --> AM
    AM --> AR
    AR --> BM
    BM --> FB
    BM --> SB
    BM --> UB
    BM --> LB
    AR --> TOP
    TOP --> SYS
    SYS --> HI
    HI --> AR
```

The intended hot path is:

1. Try tcache without locking.
2. Only on tcache miss, select and lock an arena.
3. Search arena-local structures from cheapest to most expensive.
4. Grow the heap or mmap only if all reusable structures miss.

## 3. Core Data Structure Relationships

```mermaid
classDiagram
    class ArenaManager {
      Arena main_arena_
      Arena* arena_list_
      Arena* free_list_
      atomic<int> narenas_
      int arena_limit_
      SysMemory* sys_mem_
      get_arena(size)
      new_arena(size)
      reused_arena(avoid)
    }

    class Arena {
      Chunk* top_
      Chunk* last_remainder_
      BinManager bins_
      pthread_mutex_t mutex_
      size_t system_mem_
      lock()
      unlock()
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
      alloc(idx)
      free(idx, chunk)
    }

    class Chunk {
      size_t prev_size
      size_t size
      Chunk* fd
      Chunk* bk
      Chunk* fd_nextsize
      Chunk* bk_nextsize
    }

    class HeapInfo {
      Arena* ar_ptr
      HeapInfo* prev
      size_t size
      heap_for_ptr(ptr)
      arena_for_chunk(ptr, main)
    }

    ArenaManager "1" --> "many" Arena
    Arena --> BinManager
    BinManager --> FastBins
    BinManager --> SmallBins
    BinManager --> LargeBins
    BinManager --> UnsortedBin
    FastBins --> Chunk
    SmallBins --> Chunk
    LargeBins --> Chunk
    UnsortedBin --> Chunk
    TcachePerthread --> Chunk
    HeapInfo --> Arena
```

Important ownership rules:

- `tcache` is thread-local and does not require an arena lock.
- `Arena` owns all non-tcache bin structures and must be locked before touching them.
- `Chunk` objects are not separately allocated metadata; chunk headers live inside the managed heap.
- Non-main arena chunks carry `NON_MAIN_ARENA`, and `my_free` uses `HeapInfo` to find the owning arena.

## 4. Chunk Layout

Each allocation starts with a `Chunk` header. The user pointer points after `prev_size` and `size`.

```mermaid
flowchart LR
    subgraph C["Chunk in memory"]
        P["prev_size<br/>8 bytes"]
        S["size | flags<br/>8 bytes"]
        U["user data<br/>also stores fd/bk when free"]
        N["next chunk prev_size<br/>overlaps usable area in ptmalloc model"]
    end

    P --> S --> U --> N
```

The low bits of `size` are flags:

| Flag | Meaning |
|---|---|
| `PREV_INUSE` | Previous physical chunk is considered allocated, so backward coalescing must not use `prev_size`. |
| `IS_MMAPPED` | Chunk was directly mmap'd and should be unmapped on free. |
| `NON_MAIN_ARENA` | Chunk belongs to a non-main arena; free must recover its owner through `HeapInfo`. |

Free chunks reuse the user area for intrusive list pointers:

```text
allocated chunk:
  [prev_size][size|flags][user bytes...]

free chunk:
  [prev_size][size|flags][fd][bk][fd_nextsize][bk_nextsize]...
```

`fd_nextsize` and `bk_nextsize` exist for layout compatibility, but the active large-bin implementation currently uses sorted `fd`/`bk` lists plus a binmap instead of maintaining a secondary next-size chain.

## 5. Arena and Heap Layout

Main arena memory is mapped as a heap region with one top chunk and a fencepost at the end.

```mermaid
flowchart LR
    M["mmap region"]
    T["top chunk<br/>usable wilderness"]
    F["fencepost chunk<br/>MINSIZE"]

    M --> T --> F
```

Non-main arenas place `HeapInfo` and `Arena` metadata at the beginning of a `HEAP_MAX_SIZE`-aligned heap:

```mermaid
flowchart LR
    H["HeapInfo<br/>ar_ptr -> Arena"]
    A["Arena object<br/>alignas(128)"]
    T["top chunk"]
    F["fencepost"]

    H --> A --> T --> F
```

The alignment is essential:

```text
heap_for_ptr(p) = p & ~(HEAP_MAX_SIZE - 1)
```

That mask only works if non-main heaps begin on `HEAP_MAX_SIZE` boundaries. The implementation overmaps and trims prefix/suffix pages to guarantee the alignment.

The fencepost prevents `Chunk::mark_inuse()` from writing past the mapped heap when the previous chunk reaches the region boundary.

## 6. Allocation Path

```mermaid
flowchart TD
    Start["my_malloc(size)"]
    Size["request2size(size)"]
    TInit["initialize tcache if needed"]
    TTry{"tcache hit?"}
    TRet["mark chunk in use<br/>return user pointer"]
    Reg["register thread if needed"]
    Arena["ArenaManager::get_arena<br/>lock selected arena"]
    Pipe["AllocPipeline::execute"]
    FTry{"fastbin hit?"}
    STry{"smallbin hit?"}
    UTry{"unsorted hit?"}
    LTry{"largebin hit?"}
    TopTry{"top chunk has space?"}
    Sys["sys alloc<br/>mmap or new heap"]
    Unlock["unlock arena"]
    Ret["return pointer or null"]

    Start --> Size --> TInit --> TTry
    TTry -- yes --> TRet
    TTry -- no --> Reg --> Arena --> Pipe
    Pipe --> FTry
    FTry -- no --> STry
    STry -- no --> UTry
    UTry -- no --> LTry
    LTry -- no --> TopTry
    TopTry -- no --> Sys
    FTry -- yes --> Unlock
    STry -- yes --> Unlock
    UTry -- yes --> Unlock
    LTry -- yes --> Unlock
    TopTry -- yes --> Unlock
    Sys --> Unlock --> Ret
```

The key optimization is that tcache is checked before `ArenaManager::get_arena`. A tcache hit no longer pays for arena lookup or a mutex lock.

## 7. Free Path

```mermaid
flowchart TD
    Start["my_free(ptr)"]
    Null{"ptr == null?"}
    Chunk["Chunk::from_user_ptr"]
    Mmap{"IS_MMAPPED?"}
    Unmap["SysMemory::unmap"]
    TRange{"in tcache range<br/>and tcache has room?"}
    TPut["push into tcache<br/>safe-link next pointer"]
    Owner["HeapInfo::arena_for_chunk"]
    Lock["lock owner arena"]
    Coalesce["forward/backward coalesce"]
    Top{"adjacent to top?"}
    MergeTop["merge into top"]
    Unsorted["push into unsorted bin"]
    Unlock["unlock arena"]

    Start --> Null
    Null -- yes --> Unlock
    Null -- no --> Chunk --> Mmap
    Mmap -- yes --> Unmap
    Mmap -- no --> TRange
    TRange -- yes --> TPut
    TRange -- no --> Owner --> Lock --> Coalesce --> Top
    Top -- yes --> MergeTop --> Unlock
    Top -- no --> Unsorted --> Unlock
```

Tcache chunks are treated as allocated from the arena coalescing perspective. That is why pushing a chunk to tcache does not immediately clear neighboring `PREV_INUSE` metadata.

## 8. Bin Flow

```mermaid
flowchart LR
    Free["my_free"]
    TC["Tcache<br/>thread-local"]
    FB["Fastbins<br/>tiny deferred coalescing"]
    CONS["malloc_consolidate"]
    UB["Unsorted bin<br/>staging"]
    SB["Small bins<br/>exact fit"]
    LB["Large bins<br/>best fit + binmap"]
    TOP["Top chunk"]
    SYS["System mmap/new heap"]

    Free --> TC
    Free --> UB
    FB --> CONS --> UB
    UB --> SB
    UB --> LB
    UB --> TC
    SB --> TOP
    LB --> TOP
    TOP --> SYS
```

The unsorted bin is intentionally a staging area, not a permanent store. On allocation, it scans a bounded number of entries:

- exact-size chunks can satisfy the current request;
- exact-size chunks can refill the current tcache bin;
- non-matching chunks are moved to small or large bins;
- arbitrary larger chunks are not split directly from unsorted, because that acts like first-fit and caused severe fragmentation in random workloads.

## 9. Large Bin Search

```mermaid
flowchart TD
    Req["large request size"]
    Idx["largebin_index(size)"]
    Map["BinMap::find_first_from(idx)"]
    Empty{"found occupied bin?"}
    Walk["walk sorted list<br/>smallest adequate chunk"]
    Fit{"fit found?"}
    Split["split if remainder >= MINSIZE"]
    Return["return victim"]
    Miss["miss"]

    Req --> Idx --> Map --> Empty
    Empty -- no --> Miss
    Empty -- yes --> Walk --> Fit
    Fit -- yes --> Split --> Return
    Fit -- no --> Map
```

Large bins are sorted largest-to-smallest in insertion order, and allocation walks from the back to find the smallest chunk that can satisfy the request. The binmap avoids scanning empty large-bin ranges.

## 10. What Comes From ptmalloc and What Is Custom

| Area | ptmalloc-style design | Project-specific implementation |
|---|---|---|
| Chunk metadata | Boundary tags, low-bit flags | C++ `Chunk` methods and strong `ChunkSize` wrappers |
| Tcache | Per-thread lock-free cache | Safe-linking plus static bootstrap storage |
| Fastbins | Deferred coalescing for tiny chunks | Atomic CAS stack class |
| Small bins | Exact-size bins | `SmallBins` class with intrusive FIFO lists |
| Large bins | Best-fit by size range | Sorted lists plus `BinMap`, no active `fd_nextsize` chain |
| Unsorted bin | Deferred sorting after free/coalesce | Bounded scan and exact-size tcache refill only |
| Arenas | Per-thread arena assignment | `ArenaManager` create/reuse logic with `HeapInfo` owner lookup |
| Debug checks | Development diagnostics | Compiled out unless `MY_PTMALLOC_ENABLE_DEBUG=1` |

## 11. Optimization Summary

### Tcache Before Arena Lock

Previous behavior selected and locked an arena before running the allocation pipeline, even when tcache could satisfy the request. Now `my_malloc` checks tcache first. This turns the common small-object hit path into:

```text
request size -> tcache index -> pop thread-local entry -> return
```

No arena mutex is involved.

### Per-thread Arenas

Threads are no longer pinned to `main_arena`. The manager creates arenas up to `ncpus * ARENA_MULTIPLIER`, then reuses or try-locks existing arenas. This reduces contention for workloads that miss tcache.

### Bounded Unsorted Scan

Unsorted scanning is capped by `UNSORTED_SCAN_LIMIT`. This prevents one unlucky allocation from sorting an arbitrarily long unsorted list.

### Exact Tcache Refill Only

An attempted optimization filled tcache with unrelated unsorted chunks. It improved some local hits but badly increased fragmentation and RSS in random workloads. The final design only refills tcache with exact-size chunks for the current request.

### Binmap-assisted Large Bins

Large-bin allocation now skips empty ranges with `BinMap::find_first_from`, then performs best-fit inside the selected sorted list. This is safer than restoring `fd_nextsize` before all split/unlink/coalesce paths maintain it correctly.

### Hot-path Debug Gating

Metadata validation and `fprintf` diagnostics are useful during allocator development but expensive in release builds. They are behind `MY_PTMALLOC_ENABLE_DEBUG`.

## 12. Current Performance Shape

Measured on Linux x86_64 with `-O2 -march=native` on May 4, 2026:

- same-size tcache-heavy workloads are faster than glibc in this benchmark;
- batch allocation/free is faster because most operations avoid arena locks;
- multi-threaded throughput improves after enabling per-thread arenas;
- random alloc/free/realloc is still slower than glibc;
- fragmentation-heavy realloc workloads remain the biggest gap;
- large allocation RSS remains higher because trimming and adaptive thresholds are simpler.

## 13. Remaining Work

- Implement in-place `realloc` growth by merging with the next free chunk.
- Trigger `systrim` automatically after large frees or top growth.
- Add automatic thread-exit tcache flushing.
- Restore `fd_nextsize` only with complete tests for large-bin insertion, unlink, split, and coalescing.
- Add targeted benchmarks for arena reuse, unsorted scan limits, and fragmentation.
