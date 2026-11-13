#pragma once
// conch's shared types and functions. You won't need to change this file.

#include <sys/types.h>

#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "proc.hpp"

// ---- the shell's state ----

struct Shell {
    int last_status = 0;   // $?
    bool exiting = false;  // set by exit
    int exit_code = 0;
};

// ---- words and operators: lexer.cpp ----

struct Token {
    enum Kind { Word, Op };
    Kind kind = Word;
    std::string text;  // the word, or the operator
    int fd = -1;       // a redirection's fd number, if one was written before it

    bool operator==(const Token&) const = default;
};

struct SyntaxError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Splits a line into words and redirection operators: < > >> <& >&,
// each optionally after an fd number ("2>"). A # at the start of a word
// starts a comment.
std::vector<Token> tokenize(const std::string& line);

// ---- commands: parser.cpp ----

struct Redirect {
    enum Op { Read, Write, Append, Dup };
    Op op = Read;
    int fd = 0;
    std::string path;  // Read, Write, Append
    int from = -1;     // Dup

    bool operator==(const Redirect&) const = default;
};

struct Command {
    std::vector<std::string> argv;
    std::vector<Redirect> redirects;
};

// Turns tokens into a command: its words, and its redirections with their
// targets. Throws SyntaxError for a redirection with no target, or a >& or
// <& whose target isn't an fd number.
Command parse_command(const std::vector<Token>& tokens);

// ---- running things: exec.cpp, redirect.cpp ----

// Tokenizes, parses and runs one line, updating $?. Syntax errors are
// reported with status 2.
void run_line(Shell& sh, const std::string& line);

// Runs a command: a builtin in the shell, anything else in a child.
// Returns its status.
int run_command(Shell& sh, const Command& c);

// Applies redirections to this process, in order. Throws
// std::system_error naming the path or fd.
void apply_redirects(const std::vector<Redirect>& redirects);

// Saves every fd the redirections would change, and puts them back when
// destroyed, so a builtin can be redirected inside the shell.
class SavedFds {
  public:
    explicit SavedFds(const std::vector<Redirect>& redirects);
    ~SavedFds();
    SavedFds(const SavedFds&) = delete;
    SavedFds& operator=(const SavedFds&) = delete;

  private:
    std::vector<std::pair<int, int>> saved_;  // {fd, its copy}; copy -1 if it was closed
};

// In a child: applies the redirections and runs the command (a builtin,
// or execvp). Never returns.
[[noreturn]] void exec_command(Shell& sh, const Command& c);

// Runs a builtin, or a command with no words, in the shell itself.
int run_in_shell(Shell& sh, const Command& c);

// ---- builtins: builtins.cpp ----

using Builtin = int (*)(Shell&, const std::vector<std::string>&);

// The builtin called name, or nullptr.
Builtin find_builtin(const std::string& name);
