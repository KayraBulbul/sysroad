#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <string>
#include <vector>

#include "proc.hpp"

struct Output {
    std::string out;  // everything the command wrote to stdout
    Status status;
};

// Runs argv with its stdout on a pipe, and returns what it printed and
// how it ended. Its stdin and stderr are this process's.
Output capture(const std::vector<std::string>& argv);

// Runs argv, writing `input` to its stdin while reading its stdout, so
// neither side ever waits for the other. A command that stops reading
// early mustn't kill this process.
Output communicate(const std::vector<std::string>& argv, const std::string& input);
