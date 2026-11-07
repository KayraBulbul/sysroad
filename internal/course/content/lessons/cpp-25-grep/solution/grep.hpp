#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <istream>
#include <ostream>
#include <string_view>
#include <vector>

// Prints each line of `in` that contains `pattern`, after `prefix` ("" or
// "name:"). Returns how many lines it printed.
std::size_t grep_stream(std::istream& in, std::string_view pattern, std::string_view prefix,
                        std::ostream& out);

// grep PATTERN [FILE]... Returns 0 if a line matched, 1 if none, 2 on any error.
int run_grep(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
             std::ostream& err);
