#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

struct Counts {
    std::size_t lines = 0;
    std::size_t words = 0;
    std::size_t bytes = 0;
};

struct Options {
    bool lines = false;
    bool words = false;
    bool bytes = false;
    char bad = 0;                         // an unknown flag letter, or 0
    std::vector<std::string_view> files;  // every argument that isn't a flag
};

Counts count_stream(std::istream& in);
Options parse_options(const std::vector<std::string_view>& args);

// The chosen columns, 7 wide and space separated, with no newline.
std::string format_counts(const Counts& c, const Options& o);

// The whole program. Returns the exit status.
int run_wc(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
           std::ostream& err);
