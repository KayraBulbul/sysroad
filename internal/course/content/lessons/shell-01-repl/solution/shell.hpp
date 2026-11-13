#pragma once
// conch's shared types and functions. You won't need to change this file.

#include <sys/types.h>

#include <string>
#include <vector>

#include "proc.hpp"

// ---- the shell's state ----

struct Shell {
    int last_status = 0;  // $?
};

// ---- words: lexer.cpp ----

// Splits a line into words at spaces and tabs. A # at the start of a word
// starts a comment, which runs to the end of the line.
std::vector<std::string> split_words(const std::string& line);

// ---- running things: exec.cpp ----

// Splits a line into words and runs them as a command, updating $?.
void run_line(Shell& sh, const std::string& line);

// Runs a command in a child process and waits for it. Returns its status
// as $? shows it: 127, with a message, if there's no such command.
int run_command(const std::vector<std::string>& argv);
