#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <istream>

struct Counts {
    std::size_t lines = 0;
    std::size_t words = 0;
    std::size_t bytes = 0;
};

// Counts lines, words and bytes in everything `in` produces.
Counts count_stream(std::istream& in);
