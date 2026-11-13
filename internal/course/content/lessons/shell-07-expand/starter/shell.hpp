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
    pid_t pgid = 0;                // its process group, with job control on; else 0
    std::vector<pid_t> pids;       // one per command, in order
    std::vector<bool> ended;       // which processes have ended...
    std::vector<Status> statuses;  // ...and how
    bool stopped = false;
    int stop_signal = 0;
    std::string text;  // the command line, for jobs and notifications

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
    int last_status = 0;       // $?
    bool interactive = false;  // reading commands from a terminal, with job control
    int tty = 0;               // the terminal, when interactive
    bool exiting = false;      // set by exit
    int exit_code = 0;
    std::map<std::string, std::string> vars;  // shell variables that aren't exported
    std::vector<Job> jobs;                    // background and stopped jobs
    pid_t last_background = 0;                // $!
    bool warned_stopped = false;              // exit already warned about stopped jobs
};

// ---- words and operators: lexer.cpp ----

struct Token {
    enum Kind { Word, Op };
    Kind kind = Word;
    std::string text;         // the word as written, quotes and all; or the operator
    bool assignment = false;  // a word written NAME=value, with NAME unquoted
    int fd = -1;              // a redirection's fd number, if one was written before it

    bool operator==(const Token&) const = default;
};

struct SyntaxError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Splits a line into words and operators: | || & && ; < > >> <& >&, the
// last five optionally after an fd number ("2>"). Quoted text and
// backslashed characters don't split words or make operators. A # at the
// start of a word starts a comment. Throws SyntaxError for an unclosed
// quote.
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
    std::vector<std::pair<std::string, std::string>> assignments;  // NAME=value before the command
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
// destroyed, so a builtin can be redirected inside the shell. Throws
// std::system_error if an open fd can't be copied (no fds left).
class SavedFds {
  public:
    explicit SavedFds(const std::vector<Redirect>& redirects);
    ~SavedFds();
    SavedFds(const SavedFds&) = delete;
    SavedFds& operator=(const SavedFds&) = delete;

  private:
    std::vector<std::pair<int, int>> saved_;  // {fd, its copy}; copy -1 if it was closed
};

// In a child: sets the assignments in the environment, applies the
// redirections and runs the command (a builtin, or execvp). Never returns.
[[noreturn]] void exec_command(Shell& sh, const Command& c);

// Runs a builtin, or a command with no words, in the shell itself.
int run_in_shell(Shell& sh, const Command& c);

// Starts a pipeline's processes, connected by pipes. With job control
// on, they get a process group of their own, which takes the terminal if
// the job is in the foreground.
Job launch_job(Shell& sh, const Pipeline& p);

// ---- builtins: builtins.cpp ----

using Builtin = int (*)(Shell&, const std::vector<std::string>&);

// The builtin called name, or nullptr.
Builtin find_builtin(const std::string& name);

// ---- variables: vars.cpp ----

// A shell variable, or an environment variable; nullopt if neither.
std::optional<std::string> lookup_var(const Shell& sh, const std::string& name);
// Sets a variable: in the environment if it's exported, else in sh.vars.
void set_var(Shell& sh, const std::string& name, const std::string& value);
// Moves a shell variable into the environment (or makes an empty one).
void export_var(Shell& sh, const std::string& name);
// Removes a variable from both.
void unset_var(Shell& sh, const std::string& name);
bool is_valid_name(const std::string& name);

// Expands a word as written: removes quotes and backslashes, replaces
// $NAME, ${NAME}, $?, $$ and $!, and a leading ~. nullopt if a word with
// no quotes expands to nothing, which then isn't a word at all. Throws
// SyntaxError for a ${ with no }.
std::optional<std::string> expand_word(const std::string& word, const Shell& sh);

// ---- jobs: jobs.cpp ----

// Gets the shell ready to run jobs on its terminal.
void init_job_control(Shell& sh);
// Adds a background job to the table, prints "[n] pgid" to stderr.
int start_background(Shell& sh, Job job);
// Waits for a foreground job; one that stops goes in the table.
int run_foreground(Shell& sh, Job job);
// Notices jobs that have ended, prints "[n] Done  text", and drops them.
void notify_jobs(Shell& sh);

int builtin_jobs(Shell& sh, const std::vector<std::string>& argv);
int builtin_wait(Shell& sh, const std::vector<std::string>& argv);
int builtin_fg(Shell& sh, const std::vector<std::string>& argv);
int builtin_bg(Shell& sh, const std::vector<std::string>& argv);
// Whether exit may go ahead: warns once if jobs are stopped.
bool may_exit(Shell& sh);
