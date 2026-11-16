#pragma once
// conch, a small Unix shell. The tests use Shell and run_line.

#include <string>
#include <vector>

// What the shell remembers from one line to the next.
struct Shell {
    int last_status = 0;   // $?
    bool exiting = false;  // exit has run: stop reading
    int exit_code = 0;
};

// Runs one line of input in the shell, and returns $? after it.
int run_line(Shell& sh, const std::string& line);

// Splits a line into words at spaces and tabs. A # at the start of a word
// starts a comment, which runs to the end of the line.
std::vector<std::string> split_words(const std::string& line);

// Runs a command in a child process and waits for it. Returns its status
// as $? shows it: the exit code, or 128 plus the signal that killed it.
int run_command(const std::vector<std::string>& argv);

// A builtin runs inside the shell itself, and returns its status.
using Builtin = int (*)(Shell&, const std::vector<std::string>&);

// The builtin called name, or nullptr.
Builtin find_builtin(const std::string& name);
