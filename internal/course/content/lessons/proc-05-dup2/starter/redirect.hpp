#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <optional>
#include <string>
#include <vector>

#include "proc.hpp"

// One redirection, as a shell writes it:
//   [n]<path   n (default 0) reads from path
//   [n]>path   n (default 1) writes to path, emptying it first
//   [n]>>path  n (default 1) appends to path
//   [n]>&m     n (default 1) becomes a copy of fd m
//   [n]<&m     n (default 0) becomes a copy of fd m
struct Redirect {
    enum Op { Read, Write, Append, Dup };
    int fd = 0;
    Op op = Read;
    std::string path;  // Read, Write, Append
    int from = -1;     // Dup

    bool operator==(const Redirect&) const = default;
};

// The redirection one word spells, or nullopt if it isn't one.
std::optional<Redirect> parse_redirect(const std::string& word);

// Rearranges this process's fds as r says. Throws std::system_error,
// whose what() starts with the path (or the fd m), if it can't.
void apply_redirect(const Redirect& r);

// Runs argv in a child with the redirections applied in order, and waits
// for it. If one fails, the child prints "redir: <what()>" to stderr and
// exits with 1, without running the command.
Status run_redirected(const std::vector<std::string>& argv, const std::vector<Redirect>& redirects);
