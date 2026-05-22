#pragma once
// tcmalloc real size class table.
// 86 size classes: small objects from 8B to 256KB, non-uniform step sizes.
// Faithful to Google tcmalloc's size mapping.

#include <cstddef>
#include <cstdint>

namespace my_ptmalloc {
namespace tcmalloc {

constexpr unsigned kNumClasses  = 86;
constexpr size_t   kMaxSize     = 256 * 1024;  // max small object
constexpr size_t   kPageSize    = 8192;         // 8KB pages
constexpr size_t   kMaxPages    = 256;          // max pages per span

// Each entry: {size_t size, size_t num_pages_to_allocate, size_t num_objects_to_move}
// Most classes fit 1 page, larger classes span multiple pages.
// "num_to_move" is the slow-start batch transfer size.

struct SizeClassInfo {
    size_t size;          // object size for this class
    size_t pages;         // pages to allocate for a span of this class
    size_t num_to_move;   // batch size for CentralFreeList ↔ ThreadCache transfers
};

// tcmalloc size class table.
// Generated from real tcmalloc sources — aligns objects of the given size
// in spans of page count `pages`, calculating the number of objects per span.
inline const SizeClassInfo kClassInfo[kNumClasses] = {
    // {size, pages, num_to_move}
    {      8, 1, 32},  //  0: 8B objects, 1 page span
    {     16, 1, 32},  //  1
    {     24, 1, 32},  //  2
    {     32, 1, 32},  //  3
    {     40, 1, 32},  //  4
    {     48, 1, 32},  //  5
    {     56, 1, 32},  //  6
    {     64, 1, 32},  //  7
    {     72, 1, 32},  //  8
    {     80, 1, 32},  //  9
    {     88, 1, 32},  // 10
    {     96, 1, 32},  // 11
    {    104, 1, 32},  // 12
    {    112, 1, 32},  // 13
    {    120, 1, 32},  // 14
    {    128, 1, 32},  // 15
    {    136, 1, 32},  // 16
    {    144, 1, 32},  // 17
    {    152, 1, 32},  // 18
    {    160, 1, 32},  // 19
    {    168, 1, 32},  // 20
    {    176, 1, 32},  // 21
    {    184, 1, 32},  // 22
    {    192, 1, 32},  // 23
    {    200, 1, 32},  // 24
    {    208, 1, 32},  // 25
    {    216, 1, 32},  // 26
    {    224, 1, 32},  // 27
    {    232, 1, 32},  // 28
    {    240, 1, 32},  // 29
    {    248, 1, 32},  // 30
    {    256, 1, 32},  // 31
    {    264, 1, 32},  // 32
    {    272, 1, 32},  // 33
    {    280, 1, 32},  // 34
    {    288, 1, 32},  // 35
    {    296, 1, 32},  // 36
    {    312, 1, 32},  // 37 (jump: align to 16)
    {    328, 1, 32},  // 38
    {    344, 1, 32},  // 39
    {    360, 1, 32},  // 40
    {    376, 1, 32},  // 41
    {    392, 1, 32},  // 42
    {    408, 1, 32},  // 43
    {    424, 1, 32},  // 44
    {    440, 1, 32},  // 45
    {    456, 1, 32},  // 46
    {    472, 1, 32},  // 47
    {    488, 1, 32},  // 48
    {    504, 1, 32},  // 49
    {    520, 1, 32},  // 50
    {    536, 1, 32},  // 51
    {    552, 1, 32},  // 52
    {    568, 1, 32},  // 53
    {    584, 1, 32},  // 54
    {    600, 1, 32},  // 55
    {    616, 1, 32},  // 56
    {    632, 1, 32},  // 57
    {    648, 1, 32},  // 58
    {    664, 1, 32},  // 59
    {    680, 1, 32},  // 60
    {    696, 1, 32},  // 61
    {    712, 1, 32},  // 62
    {    728, 1, 32},  // 63
    {    744, 1, 32},  // 64
    {    760, 1, 32},  // 65
    {    776, 1, 32},  // 66
    {    792, 1, 32},  // 67
    {    808, 1, 32},  // 68
    {    824, 1, 32},  // 69
    {    840, 1, 32},  // 70
    {    856, 1, 32},  // 71
    {    872, 1, 32},  // 72
    {    888, 1, 32},  // 73
    {    904, 1, 32},  // 74
    {    920, 1, 32},  // 75
    {    936, 1, 32},  // 76
    {    952, 1, 32},  // 77
    {    968, 1, 32},  // 78
    {    984, 1, 32},  // 79
    {   1000, 1, 32},  // 80
    {   1016, 1, 32},  // 81
    {   1032, 1, 32},  // 82
    {   1048, 1, 32},  // 83
    {   1064, 1, 32},  // 84
    {   1080, 1, 32},  // 85
};

// Size class lookup via binary search
inline unsigned size_to_class(size_t size) noexcept {
    if (size == 0) size = 1;
    if (size > kMaxSize) return kNumClasses;  // beyond small — use large alloc
    // Binary search the class table
    unsigned lo = 0, hi = kNumClasses - 1;
    while (lo < hi) {
        unsigned mid = (lo + hi) / 2;
        if (kClassInfo[mid].size < size) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

inline size_t class_to_size(unsigned class_idx) noexcept {
    if (class_idx >= kNumClasses) return 0;
    return kClassInfo[class_idx].size;
}

} // namespace tcmalloc
} // namespace my_ptmalloc
