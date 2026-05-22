# jemalloc — FreeBSD malloc 复现

## 来源

jemalloc 是 Jason Evans 为 FreeBSD 开发的 malloc 实现（2005年），后来被 Facebook 大规模采用并加强。它在多核系统上以低碎片和高可观测性著称。2014 年之后成为 FreeBSD libc 的默认分配器。Redis、MariaDB、Aerospike 等数据库常用 jemalloc。

## 整体架构

```
用户调用 malloc(size)

  ┌──────────────────────────────────────────────────┐
  │ tcache (每线程, LIFO stack)                      │
  │                                                  │
  │ 每线程 36 个 size class 的 LIFO stack             │
  │  ┌────┬────┬────┬─···─┬─────┐                   │
  │  │ 8B │16B │24B │     │14KB │  low_water 水位    │
  │  └────┴────┴────┴─···─┴─────┘  GC 自动 trim      │
  │       │ hit: return ptr (fast!)                  │
  │       │ miss: refill from arena bin              │
  └───────┼──────────────────────────────────────────┘
          │
          ▼
  ┌──────────────────────────────────────────────────┐
  │ Arena[0..7] (round-robin 分配, 各带 mutex)        │
  │                                                  │
  │  per-size-class bin:                             │
  │    bin_t                                         │
  │    ├── runcur (当前非满 run)                      │
  │    └── runs tree (所有非满 run, sorted)           │
  │         │                                        │
  │         ▼                                        │
  │    ┌───────── run ─────────┐                     │
  │    │ extent (连续 N 页)     │                     │
  │    │ bitmap: 每个 slot 1 bit │                    │
  │    │ nfree: 空闲 slot 数    │                     │
  │    └────────────────────────┘                    │
  │                                                  │
  │  extent tree (per arena, 4 states):               │
  │    dirty -> muzzy -> clean -> retained           │
  └───────┼──────────────────────────────────────────┘
          │
          ▼
  ┌────────────────────────────────────┐
  │ rtree (radix tree)                 │
  │ addr -> extent*  lock-free read    │
  └────────────────────────────────────┘
          │
          ▼
  ┌────────────────────────────────────┐
  │ OS: mmap / madvise                 │
  │ decay timer -> purge (MADV_DONTNEED)│
  └────────────────────────────────────┘
```

## 核心设计

### 层次结构：arena -> bin -> run

```
  ┌──────────── Arena[0] ────────────┐
  │  bin[0] (8B)                     │
  │   ├── runcur -> run A (nfree>0)  │
  │   └── runs: [run A] [run B]      │
  │  bin[1] (16B)                    │
  │   ├── runcur -> run C            │
  │   └── runs: [run C]              │
  │  ...                             │
  │  bin[35] (14336B)                │
  │                                  │
  │  extent trees:                   │
  │   dirty   -> extent₁ -> extent₂  │
  │   muzzy   -> (empty)             │
  │   clean   -> extent₃             │
  │   retained -> (empty)            │
  └──────────────────────────────────┘

  Arena[1] ... Arena[7]  (结构相同, 独立锁)
```

### 真实 size class 表（36 个 small class）

```
8,16,24,32,40,48,56,64,80,96,112,128,160,192,224,256,
320,384,448,512,640,768,896,1024,1280,1536,1792,2048,
2560,3072,3584,4096,5120,6144,7168,8192,10240,12288,14336
```

步长按组增大：delta = 8, 16, 32, 64, 128, 256, 512, 1024, ...

- 小对象：8B-14336B（36 个 class）
- 大对象：16384B+，按 4KB 页对齐
- 巨对象：> 2MB，直接 mmap

### tcache（线程缓存）

```
  Thread-1 tcache:                 Thread-2 tcache:
    stack[0] (8B): [obj1,obj2]      stack[0] (8B): [obj3]
    stack[1] (16B): []              stack[1] (16B): [obj4,obj5,obj6]
    ...                             ...
    stack[35] (14KB): []            stack[35] (14KB): []
                                     ↑
                                     │ total_count > GC_THRESH
                                     │ -> trim all to low_water
                                     │ -> flush to arena bin
```

- LIFO stack（固定数组），`count` 追踪
- `low_water`：refill 水位线
- 自动 GC：`total_count > JE_TCACHE_GC_THRESH` 触发

### run — bitmap 管理

```
  ┌─────────────────── run ─────────────────────────┐
  │ extent: 4 pages (16KB)                          │
  │ object_size: 256B                               │
  │ nslots: 64                                      │
  │ nfree: 18                                       │
  │                                                 │
  │ bitmap (8 bytes = 64 bits):                     │
  │  ┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬──···──┐    │
  │  │1│0│1│1│0│1│0│0│1│0│1│1│0│1│0│1│  ...  │    │
  │  └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴──···──┘    │
  │   ↑ ↑   ↑   ↑ ↑    每个 bit = 1 slot           │
  │   1=used 0=free                                  │
  ├────────────────────────────────────────────────┤
  │ [slot0][slot1] ... [slot63]   64 * 256B        │
  └────────────────────────────────────────────────┘
```

分配：找 bitmap 第一个 0 bit -> set 1 -> nfree--
释放：slot -> bit 清零 -> nfree++
nfree == nslots -> run 完全空闲 -> 归还 extent

### extent 状态机

```
         ┌──────────┐
         │  DIRTY   │  刚释放，含旧数据
         └────┬─────┘
    decay ────┤ (delay: dirty_decay_ms)
              ▼
         ┌──────────┐
         │  MUZZY   │  已清零但未归还 OS
         └────┬─────┘
    decay ────┤ (delay: muzzy_decay_ms)
              ▼
         ┌──────────┐
         │  CLEAN   │  未使用过 / 已 purge
         └────┬─────┘
              │ (RSS 高时)
              ▼
         ┌──────────┐
         │ RETAINED │  物理页已还 OS, 虚拟地址保留
         └────┬─────┘
              │ (内存压力)
              ▼
         munmap (彻底归还)
```

purge = madvise(MADV_DONTNEED) 释放物理页

### rtree（地址 -> extent，lock-free 读）

```
  给定指针 p:

  ┌────────── rtree ──────────┐
  │ slots[65536]              │
  │  ...                      │
  │  [idx] -> extent*(0x...)  │  <- 原子读，无锁
  │  ...                      │
  └───────────────────────────┘

  idx = (addr >> 12) % 65536
```

## 对比

| | ptmalloc | jemalloc |
|--|----------|----------|
| 元数据 | boundary-tag (8B/分配) | bitmap per run + rtree |
| 线程缓存 | 固定 7/bin | LIFO stack，可配置 |
| 页管理 | top chunk / sbrk | 伙伴 extent + dirty/muzzy |
| 归还 OS | systrim（偶尔） | decay timer（持续/预测性） |
| 碎片控制 | 弱 | 强（bin->run->extent 层次） |
| 可观测性 | malloc_stats() | mallctl 丰富的统计项 |

## 代码结构

```
classic/jemalloc/
├── size_classes.h     # 36-class 表 + binary search
├── extent.h           # extent_t 结构 + 四种状态 + 伙伴树
├── rtree.h            # radix tree (addr -> extent)
├── jemalloc.h/cpp     # 对外 API + 完整实现 (tcache, bin, run, arena)
├── jemalloc_strategy.cpp
└── test/
```

## 构建 & 测试

```bash
cd build && cmake .. && make jemalloc_classic
./classic/jemalloc/test_jemalloc_classic
./allocator_validate --strategy jemalloc
```

## 关键阅读材料

- [jemalloc.net](https://jemalloc.net/) — 官方文档
- [jemalloc GitHub](https://github.com/jemalloc/jemalloc)
- Facebook Engineering: "Scalable memory allocation using jemalloc" (2011)
- [jemalloc paper (USENIX ATC)](http://people.freebsd.org/~jasone/jemalloc/bsdcan2006/jemalloc.pdf)
