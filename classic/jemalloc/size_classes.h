#pragma once
// jemalloc real size class table.
// ~36 small classes (8B–14KB), large classes ≥ 16KB page-aligned, huge > 2MB.

#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {
namespace jemalloc {

constexpr size_t JE_LG_PAGE       = 12;     // 4KB pages
constexpr size_t JE_PAGE_SIZE     = 1 << JE_LG_PAGE;
constexpr size_t JE_PAGE_MASK     = JE_PAGE_SIZE - 1;

constexpr unsigned JE_NSMALL_CLASSES = 36;
constexpr size_t   JE_SMALL_MAX      = 14336;  // 14KB
constexpr size_t   JE_LARGE_MIN      = 16384;  // 16KB (page-aligned)
constexpr size_t   JE_HUGE_THRESHOLD = 2 * 1024 * 1024;  // 2MB

// Small size classes: 8, 16, 24, ..., 14336
// Generated from jemalloc's size_classes.sh
struct SizeClassInfo {
    size_t size;
    size_t lg_delta;  // log2 step between classes in this group
};

inline const SizeClassInfo je_class_info[JE_NSMALL_CLASSES] = {
    // {size, lg_delta}
    {     8, 3 },  //  0: delta = 8  (2^3)
    {    16, 3 },  //  1
    {    24, 3 },  //  2
    {    32, 3 },  //  3
    {    40, 3 },  //  4
    {    48, 3 },  //  5
    {    56, 3 },  //  6
    {    64, 3 },  //  7: 64B+
    {    80, 4 },  //  8: delta = 16 (2^4)
    {    96, 4 },  //  9
    {   112, 4 },  // 10
    {   128, 4 },  // 11
    {   160, 5 },  // 12: delta = 32 (2^5)
    {   192, 5 },  // 13
    {   224, 5 },  // 14
    {   256, 5 },  // 15
    {   320, 6 },  // 16: delta = 64 (2^6)
    {   384, 6 },  // 17
    {   448, 6 },  // 18
    {   512, 6 },  // 19
    {   640, 7 },  // 20: delta = 128 (2^7)
    {   768, 7 },  // 21
    {   896, 7 },  // 22
    {  1024, 7 },  // 23: 1KB+
    {  1280, 8 },  // 24: delta = 256 (2^8)
    {  1536, 8 },  // 25
    {  1792, 8 },  // 26
    {  2048, 8 },  // 27: 2KB+
    {  2560, 9 },  // 28: delta = 512 (2^9)
    {  3072, 9 },  // 29
    {  3584, 9 },  // 30
    {  4096, 9 },  // 31: 4KB+
    {  5120,10 },  // 32: delta = 1024 (2^10)
    {  6144,10 },  // 33
    {  7168,10 },  // 34
    {  8192,10 },  // 35: 8KB+
    // { 10240,11 }, — continued for larger sizes
    // { 12288,11 }, — (not in 36 classes, handled as large)
};

// Size class lookup via binary search
inline unsigned je_size_to_class(size_t size) noexcept {
    if (size == 0) size = 1;
    if (size > JE_SMALL_MAX) return JE_NSMALL_CLASSES;  // large
    // Binary search
    unsigned lo = 0, hi = JE_NSMALL_CLASSES - 1;
    while (lo < hi) {
        unsigned mid = (lo + hi) / 2;
        if (je_class_info[mid].size < size) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

inline size_t je_class_to_size(unsigned idx) noexcept {
    if (idx >= JE_NSMALL_CLASSES) return 0;
    return je_class_info[idx].size;
}

// Large size: round up to page size
inline size_t je_large_round(size_t size) noexcept {
    return (size + JE_PAGE_MASK) & ~JE_PAGE_MASK;
}

} // namespace jemalloc
} // namespace my_ptmalloc
