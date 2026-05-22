# tcmalloc — Google Thread-Caching Malloc 复现

## 来源

tcmalloc (Thread-Caching Malloc) 是 Google 开发的 malloc 实现，2005 年随 google-perftools（后来的 gperftools）发布。设计目标是高并发下的低延迟分配，广泛用于 Chrome、BigTable 等 Google 内部系统。

## 整体架构

```
用户调用 malloc(size)

  ┌──────────────────────────────────────────────────┐
  │ ThreadCache (每线程, 无锁)                       │
  │                                                  │
  │ 每线程 86 个 size class 的 LIFO freelist          │
  │  ┌────┬────┬────┬─···─┬─────┐                   │
  │  │ cl0│ cl1│ cl2│     │ cl85│  slow-start 容量   │
  │  │ 8B │16B │24B │ ... │256KB│  low_water 水位    │
  │  └────┴────┴────┴─···─┴─────┘                   │
  │       │ hit: return ptr (fast!)                  │
  │       │ miss: batch refill                       │
  └───────┼──────────────────────────────────────────┘
          │
          ▼
  ┌──────────────────────────────────────────────────┐
  │ CentralFreeList (每 class 一个, 全局, spinlock)    │
  │                                                  │
  │  class[2]:  ┌─── nonempty spans ─────┐           │
  │             │ Span 1 (24B objects)    │           │
  │             │ Span 2 (24B objects)    │           │
  │             ├─── empty spans ──────── │           │
  │             │ Span 3 (all used)       │           │
  │             └─────────────────────────┘           │
  │       batch remove_range(N) / insert_range()      │
  └───────┼──────────────────────────────────────────┘
          │ miss: need new Span
          ▼
  ┌──────────────────────────────────────────────────┐
  │ PageHeap (全局, mutex)                            │
  │                                                  │
  │  free Span lists (per page count):                │
  │    pages[1] → Span(1p) → Span(1p)                │
  │    pages[2] → Span(2p)                           │
  │    ...                                           │
  │    pages[255] → Span(255p)                       │
  │    large_set  → Span(256p+)                      │
  │                                                  │
  │  分配: first-fit → split → return Span           │
  │  释放: coalesce adjacent → free list             │
  └───────┼──────────────────────────────────────────┘
          │
          ▼
  ┌────────────────────────────────────┐
  │ OS: mmap / munmap                  │
  │ aggressive decommit: 主动归还 OS    │
  └────────────────────────────────────┘
```

## 核心设计

### 三层之间的数据流

```
  ThreadCache                  CentralFreeList              PageHeap
  (per-thread)                 (per-size-class)             (global)
  ┌──────────┐                 ┌──────────────┐            ┌────────────┐
  │ fast     │  batch refill   │              │  new span  │            │
  │ alloc ───┼──── miss ──────►│ remove_range ├── miss ───►│ alloc_span │
  │          │                 │              │            │            │
  │ fast     │  GC / flush     │              │            │            │
  │ free  ───┼── full bin ────►│ insert_range │            │            │
  │          │                 │              │            │            │
  └──────────┘                 │ span becomes  │           │ span       │
                               │ empty ────────┼── free ──►│ free       │
                               └──────────────┘            └────────────┘
```

### 真实 size class 表（86 个 class）

不同于简单的均匀步长，tcmalloc 的 class table 经过精心设计：

| class | 0-7 | 8-11 | 12-15 | 16-19 | 20-23 | 24-31 | 32-36 | 37-85 |
|-------|-----|------|-------|-------|-------|-------|-------|-------|
| 范围 | 8-64B | 72-128B | 136-256B | 264-512B | 520-1024B | 1032-2048B | 2064-4096B | 4128-262144B |
| 步长 | 8B | 8B | 8B | 8B | 8B | 16B | 16B | 变化 |

### ThreadCache — slow-start

```
class[i] 容量随时间增长:

  capacity
     ▲
  32 │                                    ┌────────
     │                                    │ (稳定)
  16 │                         ┌──────────┘
     │                         │ (增长)
   8 │              ┌──────────┘
     │              │ (增长)
   2 │──────────────┘ (slow-start 起点)
     └─────────────────────────────────────────► 时间

  每次 refill 时若 count >= 3/4 * capacity -> capacity *= 2
  最多 8192。GC 后保留 low_water = capacity / 4。
```

### PageMap radix tree

```
  给定指针 p = 0x7f...a3b8:

  page_number = p >> 13          (除以 8KB)

  ┌────── root[512] ──────┐
  │ [0]  -> Leaf[0]        │
  │ [1]  -> Leaf[1]        │     ┌──── Leaf[1024] ────┐
  │ ...                   │     │ [0] -> Span*(0x...)  │
  │ [i]  -> Leaf[i] ──────┼───> │ [1] -> Span*(0x...)  │
  │ ...                   │     │ ...                 │
  │ [511]                 │     │ [j] -> Span* <- 命中! │
  └───────────────────────┘     └─────────────────────┘

  root_idx = (page_number >> 10) & 511
  leaf_idx = page_number & 1023
  Span* = root[root_idx][leaf_idx]   <- O(1), ~0.2% overhead
```

### Span 结构

```
  ┌─────────────────── Span ──────────────────────┐
  │ start: page 42      (第 42 个 8KB 页)          │
  │ length: 4           (4 页 = 32KB)              │
  │ size_class: 5       (class 5 = 48B objects)    │
  │ state: IN_USE                                  │
  │ refcount: 87        (已分配对象数)               │
  │ freelist: obj0->obj1->obj2->...                │
  ├────────────────────────────────────────────────┤
  │ [obj0][obj1][obj2] ... [obj680]  32KB/48B=682  │
  └────────────────────────────────────────────────┘
```

### 大对象

> 256KB 直接走 PageHeap。Span 的 `size_class` 设为 `kNumClasses`，不经过 CentralFreeList，直接 munmap 归还。

### Aggressive Decommit

超过阈值的空闲 Span 主动 madvise/munmap 返还 OS，减少 RSS。

## 对比 ptmalloc

| | ptmalloc | tcmalloc |
|--|----------|----------|
| 元数据 | boundary-tag chunk (8B overhead) | Span + PageMap radix tree |
| 线程缓存 | tcache (64 bins, 固定7 entry) | ThreadCache (86 bins, slow-start 容量) |
| 锁粒度 | arena (粗粒度) | per-class spinlock (细粒度) |
| 空闲组织 | bins(fast/small/large/unsorted) | Span 页粒度管理 |
| 归还 OS | 少（top chunk grow-only） | aggressive decommit |
| 内核扩展 | 多 arena (最多64) | 单 PageHeap (靠细粒度锁) |

## 代码结构

```
classic/tcmalloc/
├── size_classes.h       # 86-class 表 + binary search 查找
├── span.h               # Span 结构 (start, length, refcount, freelist)
├── page_map.h/cpp       # 2-level radix tree page->span 映射
├── page_heap.h/cpp      # Span 管理 (alloc/free/coalesce/release)
├── central_freelist.h/cpp  # 每 class 全局中心列表 + batch transfer
├── thread_cache.h/cpp   # 每线程缓存 + slow-start + GC
├── tcmalloc.h/cpp       # 对外 API + 大对象处理
└── tcmalloc_strategy.cpp
```

## 构建 & 测试

```bash
cd build && cmake .. && make tcmalloc_classic
./classic/tcmalloc/test_tcmalloc_classic
./allocator_validate --strategy tcmalloc
```

## 关键阅读材料

- [TCMalloc: Thread-Caching Malloc](https://google.github.io/tcmalloc/) — Google 官方文档
- [TCMalloc 设计文档](https://google.github.io/tcmalloc/design.html)
- [gperftools GitHub](https://github.com/gperftools/gperftools)
