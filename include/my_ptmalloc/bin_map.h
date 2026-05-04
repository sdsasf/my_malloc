#pragma once
// BinMap: bitmap tracking which bins have chunks
// Enables O(1) "find next non-empty bin" via bit scan instructions
// 128 bits = 4 uint32 words

#include "config.h"
#include "types.h"
#include <optional>

namespace my_ptmalloc {

class BinMap {
    unsigned int map_[BINMAPSIZE]{};

public:
    void init() noexcept {
        for (auto& w : map_) w = 0;
    }

    void mark(size_t bin_idx) noexcept {
        map_[binmap_word(bin_idx)] |= binmap_bit(bin_idx);
    }

    void clear(size_t bin_idx) noexcept {
        map_[binmap_word(bin_idx)] &= ~binmap_bit(bin_idx);
    }

    [[nodiscard]] bool is_set(size_t bin_idx) const noexcept {
        return (map_[binmap_word(bin_idx)] & binmap_bit(bin_idx)) != 0;
    }

    // Find first set bit >= from_idx
    // Uses __builtin_ctz for fast bit scan
    [[nodiscard]] std::optional<size_t> find_first_from(size_t from_idx) const noexcept {
        size_t word_idx = binmap_word(from_idx);
        unsigned int bit_offset = from_idx % (sizeof(unsigned int) * 8);

        // First word: mask off bits below from_idx
        if (word_idx < BINMAPSIZE) {
            unsigned int word = map_[word_idx] & (~0u << bit_offset);
            if (word != 0) {
                return word_idx * sizeof(unsigned int) * 8 + __builtin_ctz(word);
            }
        }

        // Subsequent words: full scan
        for (size_t i = word_idx + 1; i < BINMAPSIZE; ++i) {
            if (map_[i] != 0) {
                return i * sizeof(unsigned int) * 8 + __builtin_ctz(map_[i]);
            }
        }

        return std::nullopt;
    }
};

} // namespace my_ptmalloc
