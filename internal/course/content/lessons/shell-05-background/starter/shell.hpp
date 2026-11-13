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

// ---- a running pipeline ----

struct Job {
    int number = 0;                // %1, %2, ... once it's in the job table
    std::vector<pid_t> pids;       // one per command, in order
    std::vector<bool> ended;       // which processes have ended...
    std::vector<Status> statuses;  // ...and how
    std::string text;              // the command line, for jobs and notifications

    bool done() const {
        for (bool e : ended) {
            if (!e) {
                return false;
            }
        }
        return true;
    }
};

// ---- the shell's state ----

struct Shell {
    int last_status = 0;   // $?
    bool exiting = false;  // set by exit
    int exit_code = 0;
    std::vector<Job> jobs;  // background jobs
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

// Splits a line into words and operators: | || & && ; < > >> <& >&, the
// last five optionally after an fd number ("2>"). A # at the start of a
// word starts a comment.
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

struct Pipeline {
    std::vector<Command> commands;
    bool background = false;  // ended with &
    std::string text;         // its words, for the job table
};

// One pipeline in a list, and what joins it to the next.
struct Step {
    enum Next { End, Then, And, Or };  // end of line, ; or &, &&, ||
    Pipeline pipeline;
    Next next = End;
};

using List = std::vector<Step>;

// Turns tokens into a list of pipelines. Throws SyntaxError.
List parse(const std::vector<Token>& tokens);

// ---- running things: exec.cpp, redirect.cpp, pipeline.cpp ----

// Tokenizes, parses and runs one line, updating $?. Syntax errors are
// reported with status 2.
void run_line(Shell& sh, const std::string& line);

// Runs each pipeline as && || ; and & say. Returns the last status.
int run_list(Shell& sh, const List& list);

// Runs one pipeline: builtins alone in the shell, the rest as a job.
int run_pipeline(Shell& sh, const Pipeline& p);

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

// Starts a pipeline's processes, connected by pipes. A background job's
// first command reads /dev/null, not the shell's input.
Job launch_job(Shell& sh, const Pipeline& p);

// ---- builtins: builtins.cpp ----

using Builtin = int (*)(Shell&, const std::vector<std::string>&);

// The builtin called name, or nullptr.
Builtin find_builtin(const std::string& name);

// ---- jobs: jobs.cpp ----

// Adds a background job to the table, and prints "[n] pid" to stderr.
int start_background(Shell& sh, Job job);
// Waits for every process in a foreground job; returns the last one's status.
int run_foreground(Shell& sh, Job job);
// Notices jobs that have ended, prints "[n] Done  text", and drops them.
void notify_jobs(Shell& sh);

int builtin_jobs(Shell& sh, const std::vector<std::string>& argv);
int builtin_wait(Shell& sh, const std::vector<std::string>& argv);
