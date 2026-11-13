#pragma once
// conch's shared types and functions. You won't need to change this file.

#include <sys/types.h>

#include <string>
#include <vector>

#include "proc.hpp"

// ---- the shell's state ----

struct Shell {
    int last_status = 0;   // $?
    bool exiting = false;  // set by exit
    int exit_code = 0;
};

// ---- words: lexer.cpp ----

// Splits a line into words at spaces and tabs. A # at the start of a word
// starts a comment, which runs to the end of the line.
std::vector<std::string> split_words(const std::string& line);

// ---- running things: exec.cpp ----

// Splits a line into words and runs them as a command, updating $?.
void run_line(Shell& sh, const std::string& line);

// Runs a command: a builtin in the shell, anything else in a child
// (127, with a message, if there's no such command). Returns its status.
int run_command(Shell& sh, const std::vector<std::string>& argv);

// ---- builtins: builtins.cpp ----

using Builtin = int (*)(Shell&, const std::vector<std::string>&);

// The builtin called name, or nullptr.
Builtin find_builtin(const std::string& name);
