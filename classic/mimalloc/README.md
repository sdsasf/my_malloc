# mimalloc — Microsoft Memory Allocator 复现

## 来源

mimalloc (pronounced "me-malloc") 是 Microsoft Research 于 2019 年发布的开源 malloc 实现，由 Daan Leijen 设计（他也是 Koka 语言的设计者）。它在设计上极度偏向**线程局部性**和**无锁**操作，是四个经典分配器中设计理念最激进的一个。

## 整体架构

```
用户调用 malloc(size)

  ┌────────────────────────────────────────────────┐
  │ mi_heap_t (每线程独立, 完全无共享状态)           │
  │                                                │
  │  pages_direct[256] (per-size-class 快速指针)     │
  │   ┌────┬────┬────┬─···─┬──────┐                │
  │   │ 16B│ 32B│ 48B│     │ 8KB+ │                │
  │   └──┬─┴────┴────┴─···─┴──────┘                │
  │      │                                         │
  │      ▼ page                                     │
  │  ┌──────────────────────────┐                  │
  │  │ local_free (LIFO, 最快)   │                  │
  │  │ thread_free (延迟队列)    │                  │
  │  │ xthread_free (原子 CAS)  │                  │
  │  └──────────────────────────┘                  │
  │                                                │
  │  pages / pages_free / pages_full (状态链表)      │
  └────────────────────────────────────────────────┘
          │ miss: allocate new page
          ▼
  ┌────────────────────────────────────────────────┐
  │ mi_segment_t (64KB 对齐, page 容器)              │
  │                                                │
  │  ┌─── segment header ───┐                      │
  │  │ magic │ page_count    │                      │
  │  │ pages[0..N] 元数据    │                      │
  │  ├───────────────────────┤                      │
  │  │     data area         │                      │
  │  │  (blocks for pages)   │                      │
  │  └───────────────────────┘                      │
  │                                                │
  │  segment cache: 完全释放的 segment 缓存复用       │
  └────────────────────────────────────────────────┘
          │ segment miss
          ▼
  ┌────────────────────────────────────────────┐
  │ OS: mmap 128KB -> trim to 64KB aligned     │
  │ madvise(MADV_DONTNEED) for page reset       │
  └────────────────────────────────────────────┘
```

## 核心设计

### 设计原则

1. **极端的线程局部性**：每个线程有自己独立的 heap，无需任何共享状态分配
2. **纯粹的无锁**：无 mutex、无 spinlock，仅使用原子操作（CAS）做跨线程通信
3. **元数据位置**：通过指针算术直接定位（不依赖哈希表或 radix tree）

### 指针算术元数据定位（核心创新）

```
给定任意指针 p = 0x7f8a4b3c2000:

                      p
                      │
  ┌───────────────────┼───────────────────────────┐
  │ ...               ▼                           │
  │     ┌──────────────────────────────┐          │
  │     │ user block (e.g. 48 bytes)   │          │
  │     └──────────────────────────────┘          │
  │                                               │
  ├───────────────────────────────────────────────┤
  │              segment (64KB)                    │
  └───────────────────────────────────────────────┘
        ▲
        │ segment = p & ~0xFFFF   (纯位运算!)
        │ page = segment->pages[index]
        │ 不需要哈希表, 不需要树, O(1) 且无锁

  对比:
    ptmalloc:  遍历 HeapInfo 链表                    (O(n))
    tcmalloc:  PageMap radix tree 2-level 查找       (O(1), 有锁)
    jemalloc:  rtree radix tree lookup               (O(1), lock-free)
    mimalloc:  位运算 + 数组索引                      (O(1), 最快)
```

### 每线程独立 heap

```
  ┌────────── Thread-1 heap ──────────┐
  │ heap_id: 1                        │
  │ cookie:  0x9ABC3F71              │
  │                                    │
  │ pages_direct[0..255]:              │
  │   [0] -> page(16B)   (非满)       │
  │   [1] -> nullptr     (需新分配)    │
  │   [2] -> page(48B)   (非满)       │
  │   ...                             │
  │                                    │
  │ pages: page₁ <-> page₂ <-> page₃  │
  │ pages_free: page₄                 │
  │ pages_full: page₅                  │
  └────────────────────────────────────┘

  ┌────────── Thread-2 heap ──────────┐
  │ (完全独立, 无共享)                  │
  └────────────────────────────────────┘
```

### page 三层 free list

```
  ┌────────────────── mi_page_t ──────────────────┐
  │ block_size: 48                                 │
  │ capacity: 1365     (64KB / 48B)               │
  │ used: 847                                      │
  │ cookie: 0xABCD1234  (XOR 编码密钥)             │
  │                                                │
  │ ┌─ local_free ─────────────────────┐           │
  │ │ obj₁₀ -> obj₂₃ -> obj₄₅ -> ...  │ LIFO     │
  │ │ (本线程分配/释放, 最快路径)       │          │
  │ └─────────────────────────────────┘           │
  │         ▲ drain                                │
  │ ┌─ thread_free ────────────────────┐           │
  │ │ obj₈₈ -> obj₉₉ -> ...           │ LIFO     │
  │ │ (本线程释放的暂存区, 延迟合并)     │          │
  │ └─────────────────────────────────┘           │
  │         ▲ drain (atomic exchange)              │
  │ ┌─ xthread_free (std::atomic) ─────┐           │
  │ │ obj₁₂ -> obj₃₄ -> ...           │ 原子 CAS │
  │ │ (其他线程释放 → 当前线程)         │          │
  │ └─────────────────────────────────┘           │
  └───────────────────────────────────────────────┘
```

### 跨线程释放（remote-free queue）— 最独特的设计

```
  线程 A (owner of page P):           线程 B (frees P's object):
  ┌──────────────────────┐           ┌──────────────────────────┐
  │ page P               │           │ mi_free(obj)              │
  │ owner_heap = A       │           │                           │
  │ local_free: [...]    │           │ page = segment->page_of() │
  └──────────────────────┘           │ heap_tag = page->owner    │
           ▲                         │                           │
           │                         │ if heap_tag != my_heap:   │
           │  drain (下次 malloc时)    │   CAS push obj ->         │
           │  exchange(nullptr)       │   page->xthread_free     │
           │                          │   (原子, 永不阻塞!)       │
           │                          └──────────────────────────┘
           │
  关键: 释放线程永不阻塞, 分配线程按需 drain。
  这是纯粹的消息传递语义 -- 释放方"告知"分配方,
  而不是抢夺锁。
```

### 安全特性：free list 编码

```
  cookie = page->cookie  (随机数, 每个 page 不同)

  存储: block->next = real_next XOR cookie
  读取: real_next = block->next XOR cookie

  ┌──────┐    ┌──────┐    ┌──────┐
  │ obj0 │ -> │ obj1 │ -> │ obj2 │ -> ...
  └──┬───┘    └──┬───┘    └──────┘
     │next       │next
     │=obj1^C    │=obj2^C

  防止 free list corruption exploit:
  如果 attacker 覆盖了 free list 指针,
  XOR cookie 会使解码结果不可控 -> 大概率 crash。
```

### segment 布局

```
  ┌──────────────── 64KB segment ────────────────┐
  │ mi_segment_t header (magic, page_count, ...)  │
  ├──────────────────────────────────────────────┤
  │ mi_page_t pages[0]  (元数据)                  │
  │ mi_page_t pages[1]                           │
  │ ...                                          │
  ├──────────────────────────────────────────────┤
  │ data area:                                   │
  │ ┌─── page[0] blocks ───┐                    │
  │ │ [obj₀][obj₁]...[objₙ] │ (same size class)  │
  │ ├─── page[1] blocks ───┤                    │
  │ │ [obj₀][obj₁]...[objₘ] │ (different class)  │
  │ └──────────────────────┘                    │
  └──────────────────────────────────────────────┘
```

### page reset（归还物理页）

完全空闲的 page 通过 `madvise(MADV_DONTNEED)` 释放物理页 -> RSS 降低，虚拟地址保留（可快速重新 commit）。

## 对比

| | ptmalloc | mimalloc |
|--|----------|----------|
| 线程结构 | tcache(64bin) + arena 锁 | 每个线程独立 heap |
| 跨线程释放 | lock arena -> consolidate | CAS push -> drain（消息传递）|
| 元数据定位 | HeapInfo 链表遍历 | 指针位运算 O(1) |
| 锁 vs 无锁 | arena mutex, fastbin CAS | 全部原子操作，无锁 |
| 碎片 | 合并分摊 | page reset (物理页) |
| 安全 | double-free 检测 | XOR cookie + guard page |

## 代码结构

```
classic/mimalloc/
├── types.h           # mi_heap_t, mi_page_t, mi_segment_t, mi_block_t
├── os.h/cpp          # OS 操作 (mmap 64KB 对齐, madvise)
├── segment.h/cpp     # Segment 分配/释放 + segment cache + page 查找
├── mimalloc.h/cpp    # 对外 API + 完整实现 (heap/page/free list/remote queue)
├── mimalloc_strategy.cpp
└── test/
```

## 构建 & 测试

```bash
cd build && cmake .. && make mimalloc_classic
./classic/mimalloc/test_mimalloc_classic
./allocator_validate --strategy mimalloc
```

## 关键阅读材料

- [mimalloc GitHub](https://github.com/microsoft/mimalloc) — 官方仓库
- Daan Leijen et al., "Mimalloc: Free List Sharding in Arenas" (2019)
- [mimalloc 技术报告 MSR-TR-2019-18](https://www.microsoft.com/en-us/research/publication/mimalloc-free-list-sharding-in-arenas/)
- [Memory Allocation Strategies (Ginger Bill)](https://www.gingerbill.org/article/2019/02/01/memory-allocation-strategies-002/) — 各经典分配器对比
