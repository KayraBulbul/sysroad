#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <array>
#include <cstddef>
#include <string_view>

struct Histogram {
    std::array<std::size_t, 256> counts{};

    // Counts every byte in `bytes`, adding to what's there.
    void add(std::string_view bytes);

    // How many times byte b has been seen.
    std::size_t count(unsigned char b) const;

    // Every byte seen so far.
    std::size_t total() const;

    // The byte seen most often; the smallest one on a tie; 0 when empty.
    unsigned char most_common() const;
};
