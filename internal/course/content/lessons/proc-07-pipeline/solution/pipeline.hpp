#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <string>
#include <vector>

#include "proc.hpp"

using Command = std::vector<std::string>;

// Splits words into commands at each "|" word. No words, no commands.
// Throws std::invalid_argument if a command would be empty.
std::vector<Command> split_pipeline(const std::vector<std::string>& words);

// Runs the commands at the same time, each one's stdout piped into the
// next one's stdin. The first reads this process's stdin, and the last
// writes to its stdout. Waits for all of them and returns how each ended,
// in order.
std::vector<Status> run_pipeline(const std::vector<Command>& commands);
