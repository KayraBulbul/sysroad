#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <map>
#include <string>
#include <vector>

// One rule: how to make target from its dependencies.
struct Rule {
    std::string target;
    std::vector<std::string> deps;
    std::string command;
};

// Every rule, by target.
using Rules = std::map<std::string, Rule>;

// When each file was last changed. A file that doesn't exist isn't listed.
using Mtimes = std::map<std::string, long>;

// The commands to run, in order, to bring goal up to date. Each command runs
// at most once. Throws std::runtime_error with "no rule to make NAME" for a
// missing file that no rule makes, and "cycle: a -> b -> a" for a loop.
std::vector<std::string> plan(const Rules& rules, const Mtimes& mtimes, const std::string& goal);
