#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <istream>
#include <ostream>
#include <string_view>
#include <vector>

// Copies `in` to `out` one line at a time.
void cat_lines(std::istream& in, std::ostream& out);

// The whole program: each file in turn ("-" or no files means `in`).
// Returns the exit status.
int run_cat(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
            std::ostream& err);
