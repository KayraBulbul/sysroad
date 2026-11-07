#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

struct Counts {
    std::size_t lines = 0;
    std::size_t words = 0;
    std::size_t bytes = 0;
};

// Which columns to print.
struct Options {
    bool lines = false;
    bool words = false;
    bool bytes = false;
    char bad = 0;  // an unknown flag letter, or 0
};

Counts count_stream(std::istream& in);

// Reads flags like -l, -w, -c and bundles like -lw. No flags means all three.
Options parse_options(const std::vector<std::string_view>& args);

// The chosen columns, 7 wide, space separated, ending in '\n'.
std::string format_counts(const Counts& c, const Options& o);
