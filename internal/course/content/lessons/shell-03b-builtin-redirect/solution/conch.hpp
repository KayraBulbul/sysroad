#pragma once
// conch, a small Unix shell. The tests use Shell and run_line.

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// What the shell remembers from one line to the next.
struct Shell {
    int last_status = 0;   // $?
    bool exiting = false;  // exit has run: stop reading
    int exit_code = 0;
};

// Runs one line of input in the shell, and returns $? after it.
int run_line(Shell& sh, const std::string& line);

// A word, or an operator: < > >> <& >&, with the fd number written
// before it, if any ("2>").
struct Token {
    enum Kind { Word, Op };
    Kind kind = Word;
    std::string text;
    int fd = -1;
};

struct SyntaxError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Splits a line into tokens. A # at the start of a word starts a comment.
// Throws SyntaxError for an fd number too big to be one.
std::vector<Token> tokenize(const std::string& line);

struct Redirect {
    enum Op { Read, Write, Append, Dup };
    Op op = Read;
    int fd = 0;
    std::string path;  // Read, Write, Append
    int from = -1;     // Dup: the fd to copy
};

struct Command {
    std::vector<std::string> argv;
    std::vector<Redirect> redirects;
};

// Turns tokens into a command. Throws SyntaxError.
Command parse_command(const std::vector<Token>& tokens);

// Applies redirections to this process, in order. Throws
// std::system_error naming the file or fd.
void apply_redirects(const std::vector<Redirect>& redirects);

// Saves every fd the redirections would change, and puts them back when
// destroyed, so a builtin can be redirected inside the shell. Throws
// std::system_error if an open fd can't be copied (no fds left).
class SavedFds {
  public:
    explicit SavedFds(const std::vector<Redirect>& redirects);
    ~SavedFds();
    SavedFds(const SavedFds&) = delete;
    SavedFds& operator=(const SavedFds&) = delete;

  private:
    std::vector<std::pair<int, int>> saved_;  // {fd, its copy}; -1 if it was closed
};

// Runs a command in a child process, with its redirections, and waits for
// it. Returns its status as $? shows it: the exit code, or 128 plus the
// signal that killed it.
int run_command(const Command& c);

// A builtin runs inside the shell itself, and returns its status.
using Builtin = int (*)(Shell&, const std::vector<std::string>&);

// The builtin called name, or nullptr.
Builtin find_builtin(const std::string& name);
