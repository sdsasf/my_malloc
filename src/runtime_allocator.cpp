// Runtime allocator selection layer for the teaching allocator lab.

#include "my_ptmalloc/runtime_allocator.h"
#include "my_ptmalloc/allocator_lab.h"

namespace my_ptmalloc {

RuntimeAllocatorKind runtime_select_allocator(size_t) noexcept {
    switch (allocator_mode()) {
        case AllocMode::Hybrid:
            return RuntimeAllocatorKind::Hybrid;
        case AllocMode::PtmallocOnly:
            return RuntimeAllocatorKind::Ptmalloc;
        case AllocMode::TcmallocLike:
            return RuntimeAllocatorKind::TcmallocLike;
        case AllocMode::JemallocLike:
            return RuntimeAllocatorKind::JemallocLike;
        case AllocMode::MimallocLike:
            return RuntimeAllocatorKind::MimallocLike;
        case AllocMode::Adaptive:
            // The independent adaptive backend is handled directly in the
            // malloc/free/realloc frontends and never reaches this lab router.
            return RuntimeAllocatorKind::Hybrid;
    }
    return RuntimeAllocatorKind::Hybrid;
}

const char* runtime_allocator_name(RuntimeAllocatorKind kind) noexcept {
    switch (kind) {
        case RuntimeAllocatorKind::Hybrid:
            return "hybrid";
        case RuntimeAllocatorKind::Ptmalloc:
            return "ptmalloc";
        case RuntimeAllocatorKind::TcmallocLike:
            return "tcmalloc_like";
        case RuntimeAllocatorKind::JemallocLike:
            return "jemalloc_like";
        case RuntimeAllocatorKind::MimallocLike:
            return "mimalloc_like";
    }
    return "unknown";
}

bool runtime_mode_uses_family_allocators() noexcept {
    switch (allocator_mode()) {
        case AllocMode::TcmallocLike:
        case AllocMode::JemallocLike:
        case AllocMode::MimallocLike:
            return true;
        case AllocMode::Hybrid:
        case AllocMode::PtmallocOnly:
        case AllocMode::Adaptive:
            return false;
    }
    return false;
}

bool runtime_mode_is_adaptive() noexcept {
    return allocator_mode() == AllocMode::Adaptive;
}

} // namespace my_ptmalloc
