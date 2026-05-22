# ptmalloc — glibc malloc 复现

## 来源

ptmalloc 是 glibc 中 `malloc(3)` 的实现，由 Doug Lea 的 dlmalloc 派生而来，Wolfram Gloger 添加了多线程支持（`p` 代表 POSIX threads）。

这是 Linux 上使用最广泛的 malloc 实现，每个 C/C++ 程序通过 `malloc(3)` 调用的就是它。

## 整体架构

```
用户调用 malloc(size)
        │
   thread-local tcache          ← 最快路径，无锁
        │ (miss)
   ┌─ arena bins ─────────────────────────────────────┐
   │  fastbins[10]  LIFO 单链  CAS 无锁  (16-80B)     │
   │      │ (miss)                                     │
   │  smallbins[64] FIFO 双向链  精确匹配 (32-1008B)    │
   │      │ (miss)                                     │
   │  unsorted bin  FIFO 双向链  扫描 + 归类缓存        │
   │      │ (miss)                                     │
   │  largebins[63] sorted best-fit (1024B+)            │
   │      │ (miss)                                     │
   │  top chunk     wilderness  split                  │
   │      │ (exhausted)                                │
   └──────┼────────────────────────────────────────────┘
          │
   sys_alloc: mmap (small/normal) / extend heap (large arena)
          │
          ▼
   返回 user pointer
```

## 核心设计

### 边界标记 (boundary-tag) chunk

chunk 是 ptmalloc 管理内存的基本单位。相邻 chunk 之间通过 header 串联成隐式双向链表：

```
          ┌───────── in-use chunk ───────┐  ┌──── free chunk ────────────────┐
  ... ───┬────────────┬──────────────┬───┤  ├──────────┬──────────┬──────────┼───
         │ prev_size  │ size | flags │   │  │ prev_sz  │ size|P=0 │ fd (fwd) │
         │ (prev free │ (this chunk) │ user data       │          │ bk (bck) │
         │  chunk sz) │     | P=1    │   │  │          │          │          │
  ... ───┴────────────┴──────────────┴───┤  ├──────────┴──────────┴──────────┼───
         ◄── chunk A ──►                 │  │◄─────── chunk B (free) ────────►│
                        ◄── chunk C (user data starts here,                   │
                             prev_size of C is B's size,                      │
                             reused as B's user data = boundary-tag trick)    │
                                          └──────────────────────────────────┘
```

关键技巧：chunk A 的 `user data` 最后 8 字节与 chunk B 的 `prev_size` 重叠复用——这就是边界标记合并 O(1) 的原理。

### bins 体系与分配流程

```
                    ┌──────────────────────────────────────┐
                    │           ARENA (带锁)                │
                    │                                      │
  malloc(size)      │  ┌──── fastbins ────┐  LIFO, CAS    │
     │              │  │ [0]→[32B]        │  不合并        │
     ▼              │  │ [1]→[48B]        │               │
  tcache ──miss──► │  │ ...              │               │
  (64bin,7entry)   │  │ [9]→[160B]       │               │
     │              │  └──────────────────┘               │
     │(hit,fast)    │         │ miss                       │
     │              │         ▼                            │
     ▼              │  ┌──── unsorted bin ────┐  FIFO     │
  return ptr        │  │ 最近释放的 chunk 缓存  │  扫描归类  │
                    │  └──────────────────────┘           │
                    │         │ miss                       │
                    │    ┌────┴──────────┐                │
                    │    ▼               ▼                │
                    │ smallbins[64]   largebins[63]       │
                    │ (exact-fit)     (best-fit+split)     │
                    │    │               │                │
                    │    └───────┬───────┘                │
                    │            │ miss                    │
                    │            ▼                         │
                    │    ┌──── top chunk ────┐            │
                    │    │ wilderness: 切分    │            │
                    │    └───────────────────┘            │
                    └──────────────┬───────────────────────┘
                                   │ exhausted
                                   ▼
                    ┌──────────────────────────┐
                    │ sys_alloc (mmap/heap ext) │
                    │ or direct mmap >128KB     │
                    └──────────────────────────┘
```

### tcache（线程缓存，glibc 2.26+）

```
Thread-1 tcache:              Thread-2 tcache:
  bin[0] -> obj1 -> obj2        bin[0] -> obj3
  bin[1] -> (empty)             bin[1] -> obj4 -> obj5 -> obj6
  ...                           ...
  bin[63]                       bin[63]
  ┌──────────────┐              ┌──────────────┐
  │ max 7/bin    │              │ 每线程独立    │
  │ LIFO 单链表  │              │ 无锁分配     │
  └──────────────┘              └──────────────┘
           │ flush (bin满时)           │
           └──────────┬────────────────┘
                      ▼
                 Arena bins
```

- 每线程 64 个 bin，每个 bin 最多 7 个 entry
- LIFO 单链表，lock-free
- 不合并，仅快速的分配/释放路径

### 多 arena

```
进程内存布局:
┌──────────────────────────────────────────────────┐
│ main arena (mmap heap)                            │
│  ┌──────┐  ┌──────┐  ┌──────────────────────┐    │
│  │ bins │  │ top  │  │ HeapInfo → HeapInfo   │    │
│  └──────┘  └──────┘  └──────────────────────┘    │
│                          mmap heap blocks chain   │
├──────────────────────────────────────────────────┤
│ non-main arena 1 (mmap, created on contention)    │
│  ┌──────┐  ┌──────┐  ┌──────────────────────┐    │
│  │ bins │  │ top  │  │ HeapInfo → HeapInfo   │    │
│  └──────┘  └──────┘  └──────────────────────┘    │
├──────────────────────────────────────────────────┤
│ non-main arena 2 ...                     最多64个 │
└──────────────────────────────────────────────────┘

线程分配 arena 策略:
  Thread-0  ───► arena[0] (main)
  Thread-1  ───► arena[0] → lock失败 → new arena[1]
  Thread-2  ───► arena[1] → lock失败 → arena[2] or reuse
  Thread-3  ───► arena[1] (复用空闲的)
```

### 合并 (coalescing)

```
释放 ptr，指向 chunk X:

  检查前一个 chunk（prev_inuse=0?）:
    ┌─── prev_free ──┬────── X ──────┐
    │   (free)       │   (freeing)   │  ← prev_size 定位，O(1)
    └────────────────┴───────────────┘
              ⇒ unlink prev + 合并为一个大 chunk

  检查后一个 chunk（next in bins?）:
    ┌────── X ──────┬─── next_free ──┐
    │   (freeing)   │    (free)      │
    └───────────────┴────────────────┘
              ⇒ unlink next + 合并

  检查下一个 chunk 是否是 top：
    ┌────── X ──────┬────── top chunk ────────┐
    │   (freeing)   │    (wilderness)          │
    └───────────────┴──────────────────────────┘
              ⇒ 直接把 X 合并进 top，扩大 wilderness
```

`malloc_consolidate` 将 fastbins 全部移入 unsorted bin 进行合并归类。

### mmap 阈值

- 请求 > `DEFAULT_MMAP_THRESHOLD`(128KB) → 直接 mmap/munmap，不经过 arena bins
- 阈值动态调整：连续使用 mmap → 提高阈值；连续使用 heap → 降低阈值

### systrim

top chunk 过大时通过 madvise(MADV_DONTNEED) 归还物理页给 OS。

## 代码结构

```
classic/ptmalloc/
├── config.h          # 常量定义 (MINSIZE, NFASTBINS, 阈值等)
├── chunk.h           # Chunk 结构与操作 (边界标记, size 计算, bin 索引)
├── tcache.h/cpp      # 每线程 tcache (64 bin, 7 entry)
├── bins.h/cpp        # Fastbins/Smallbins/Largebins/UnsortedBin/BinMap
├── arena.h/cpp       # Arena 结构 (lock, top, last_remainder)
├── arena_manager.h/cpp  # 多 arena 管理 (创建, 复用, 线程分配)
├── heap.h/cpp        # HeapInfo (non-main arena 堆块管理)
├── sys_memory.h/cpp  # OS 内存操作 (mmap/munmap/madvise)
├── coalesce.h/cpp    # 合并逻辑 (malloc_consolidate, 前后合并, systrim)
└── ptmalloc.h/cpp    # 对外 API (pt_malloc/pt_free/pt_realloc/...)
```

## 构建 & 测试

```bash
# 构建
cd build && cmake .. && make ptmalloc_classic

# 运行测试
./classic/ptmalloc/test_ptmalloc_classic

# 通过统一验证框架
./allocator_validate --strategy ptmalloc
```

## 关键阅读材料

- [glibc malloc 源码](https://sourceware.org/git/?p=glibc.git;a=tree;f=malloc;hb=HEAD)
- Doug Lea, "A Memory Allocator" (dlmalloc 设计文档)
- glibc wiki: [Malloc Internals](https://sourceware.org/glibc/wiki/MallocInternals)
