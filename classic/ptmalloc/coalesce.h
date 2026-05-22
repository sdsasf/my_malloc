#pragma once
// Coalescing operations: malloc_consolidate, forward/backward merge, systrim.

#include "config.h"
#include "arena.h"
#include "chunk.h"

namespace my_ptmalloc {
namespace ptmalloc {

struct SysMemory;

// ─── malloc_consolidate ───
// Consolidate all fastbin chunks into the unsorted bin.
// This merges adjacent free fastbin chunks and places them in the
// unsorted bin, where they may be further coalesced or sorted into
// small/large bins.
void malloc_consolidate(Arena& arena) noexcept;

// ─── consolidate_and_free ───
// Free a chunk, merging with adjacent free chunks and top chunk.
// p must already be marked as free (PREV_INUSE set on next chunk).
void consolidate_and_free(Arena& arena, Chunk* p) noexcept;

// ─── systrim ───
// Trim the top chunk if it's too large, releasing memory back to the OS.
// Called after a free when the top chunk grows significantly.
bool systrim(Arena& arena, size_t pad) noexcept;

} // namespace ptmalloc
} // namespace my_ptmalloc
