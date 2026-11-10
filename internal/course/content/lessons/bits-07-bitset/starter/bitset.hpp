#pragma once
// The class, and what each function does. You won't need to change this file.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// A set of bits whose size is chosen when it's made. The bits are numbered
// 0 to size() - 1 and packed 64 to a word.
class Bitset {
  public:
    // size bits, all 0.
    explicit Bitset(std::size_t size);

    std::size_t size() const {
        return size_;
    }

    // One bit. These three throw std::out_of_range when i >= size().
    bool test(std::size_t i) const;
    void set(std::size_t i, bool value = true);
    void flip(std::size_t i);

    // Make every bit 1.
    void set_all();

    // How many bits are 1, and whether any is.
    std::size_t count() const;
    bool any() const;

    // The index of the lowest bit that is 1, or size() if none is.
    std::size_t find_first() const;

    // The index of the lowest bit above i that is 1, or size() if none is.
    // i can be anything, including size() or more.
    std::size_t find_next(std::size_t i) const;

    // Combine with another set of the same size, bit by bit. They throw
    // std::invalid_argument when the sizes differ.
    Bitset& operator&=(const Bitset& other);
    Bitset& operator|=(const Bitset& other);
    Bitset& operator^=(const Bitset& other);

    // A copy with every bit flipped.
    Bitset operator~() const;

    // Same size and the same bits. This compares the members as they are, so
    // it only works if the unused bits of the last word are always 0.
    bool operator==(const Bitset&) const = default;

    // The bits as '0' and '1', bit 0 first: "0110" has bits 1 and 2 set.
    std::string to_string() const;

  private:
    std::size_t size_;
    std::vector<std::uint64_t> words_;  // bit i is bit i % 64 of words_[i / 64]
};
