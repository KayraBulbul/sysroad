#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

struct GrepOptions {
    bool ignore_case = false;   // -i
    bool invert = false;        // -v: select the lines that don't match
    bool line_numbers = false;  // -n
    bool count = false;         // -c: print how many lines were selected
    char bad = 0;               // an unknown flag letter, or 0
    bool has_pattern = false;
    std::string pattern;  // a fixed string, not a regex
    std::vector<std::string_view> files;
};

// Flags anywhere; the first other argument is the pattern, the rest are files.
GrepOptions parse_grep_options(const std::vector<std::string_view>& args);

// Whether grep selects this line (it contains the pattern, or with -v doesn't).
bool matches(std::string_view line, const GrepOptions& o);

// Prints the selected lines of `in` (or with -c, their count), each after
// `prefix` ("" or "name:"). Returns how many lines were selected.
std::size_t grep_stream(std::istream& in, std::string_view prefix, const GrepOptions& o,
                        std::ostream& out);

// The whole program. Returns 0 if a line was selected, 1 if none, 2 on any error.
int run_grep(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
             std::ostream& err);
