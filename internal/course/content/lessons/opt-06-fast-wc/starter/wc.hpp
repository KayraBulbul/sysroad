#pragma once
// The course's wc from lesson 1.1.7, counting through count_fd. You won't
// need to change this file.

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "count.hpp"

struct Options {
    bool lines = false;
    bool words = false;
    bool bytes = false;
    char bad = 0;                         // an unknown flag letter, or 0
    std::vector<std::string_view> files;  // every argument that isn't a flag
};

Options parse_options(const std::vector<std::string_view>& args);
int column_width(const Options& o);
std::string format_counts(const Counts& c, const Options& o, int width);

// The whole program, reading stdin from in_fd. Returns the exit status.
int run_wc(const std::vector<std::string_view>& args, int in_fd, std::ostream& out,
           std::ostream& err);
