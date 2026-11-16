#pragma once
// conch, a small Unix shell. The tests use Shell and run_line.

#include <sys/types.h>
#include <termios.h>

#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// A running pipeline.
struct Job {
    int number = 0;            // %1, %2, ... in the job table
    pid_t pgid = 0;            // its process group, with job control; else 0
    std::vector<pid_t> pids;   // one per command, in order
    std::vector<bool> ended;   // which processes have ended...
    std::vector<int> statuses; // ...and how, as waitpid said
    std::string text;          // the command line, for jobs and notices
    bool stopped = false;
    int stop_signal = 0;
    bool has_modes = false;
    termios modes{};  // the terminal as the job left it when it stopped

    bool done() const {
        for (bool e : ended) {
            if (!e) {
                return false;
            }
        }
        return true;
    }
};

// What the shell remembers from one line to the next.
struct Shell {
    int last_status = 0;   // $?
    bool exiting = false;  // exit has run: stop reading
    int exit_code = 0;
    std::vector<Job> jobs;     // background and stopped jobs
    bool interactive = false;  // reading a terminal, with job control
    int tty = 0;               // the terminal, when interactive
    bool warned_stopped = false;  // exit already warned about stopped jobs
    pid_t last_background = 0;    // $!
    std::map<std::string, std::string> vars;  // shell variables that aren't exported
    std::vector<std::string> builtin_set;     // variables the running builtin set itself
    std::vector<std::string> history;         // the lines typed, oldest first
};

// Runs one line of input in the shell, and returns $? after it.
int run_line(Shell& sh, const std::string& line);

// A builtin runs inside the shell itself, and returns its status.
using Builtin = int (*)(Shell&, const std::vector<std::string>&);

// The builtin called name, or nullptr.
Builtin find_builtin(const std::string& name);

// A word, or an operator: | || & && ; or a redirection < > >> <& >&, with
// the fd number written before it, if any ("2>").
struct Token {
    enum Kind { Word, Op };
    Kind kind = Word;
    std::string text;
    int fd = -1;
    bool assignment = false;  // a word written NAME=value, with NAME unquoted
};

struct SyntaxError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Splits a line into tokens. Quoted text and backslashed characters don't
// end words or make operators, and stay in the word as written. A # at the
// start of a word starts a comment. Throws SyntaxError for an unclosed
// quote, or an fd number too big to be one.
std::vector<Token> tokenize(const std::string& line);

struct Redirect {
    enum Op { Read, Write, Append, Dup };
    Op op = Read;
    int fd = 0;
    std::string path;  // Read, Write, Append
    int from = -1;     // Dup: the fd to copy
};

struct Command {
    std::vector<std::pair<std::string, std::string>> assignments;  // NAME=value before the words
    std::vector<std::string> argv;
    std::vector<Redirect> redirects;
};

// Commands joined by |.
struct Pipeline {
    std::vector<Command> commands;
    bool background = false;  // ended with &
    std::string text;         // its tokens, for the job table
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

// Runs each pipeline as ; && and || say. Returns the last status.
int run_list(Shell& sh, const List& list);

// Runs one pipeline: a lone builtin in the shell, the rest in children
// joined by pipes. Returns the last command's status.
int run_pipeline(Shell& sh, const Pipeline& p);

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

// Runs a builtin, or a command with no words, in the shell itself: its
// assignments and redirections apply for as long as it runs.
int run_in_shell(Shell& sh, const Command& c);

// In a child: applies the command's redirections and runs it, a builtin or
// a program. Never returns.
[[noreturn]] void exec_command(Shell& sh, const Command& c);

// $? for a status from waitpid: the exit code, or 128 plus the signal.
int shell_status(int wstatus);


// Starts a pipeline's commands, joined by pipes. Throws std::system_error
// (and starts nothing) if a pipe or fork fails.
Job launch_job(Shell& sh, const Pipeline& p);

// Hands a job the terminal and waits for every process, or for one to
// stop. Returns the last one's status; a stopped job goes in the table.
int run_foreground(Shell& sh, Job job);

// Puts a job in the table and says "[n] pid" on stderr. Returns 0.
int start_background(Shell& sh, Job job);

// Notices jobs that have ended, says so ("[n] Done  text"), and drops them.
void notify_jobs(Shell& sh);

int builtin_jobs(Shell& sh, const std::vector<std::string>& argv);
int builtin_wait(Shell& sh, const std::vector<std::string>& argv);
int builtin_fg(Shell& sh, const std::vector<std::string>& argv);
int builtin_bg(Shell& sh, const std::vector<std::string>& argv);

// Whether exit may go ahead: warns once if jobs are stopped, and the
// second time hangs them up.
bool may_exit(Shell& sh);

// "SIGTERM" for SIGTERM, and so on; "signal N" for the rest.
std::string signal_name(int sig);

// Gets the shell ready to run jobs on its terminal: in the foreground,
// ignoring the keyboard's signals, leading its own process group.
void init_job_control(Shell& sh);

// Expands a word as written: removes quotes and backslashes, replaces
// $NAME, ${NAME}, $?, $$ and $!, and a leading ~. nullopt if a word with no
// quotes expands to nothing, which then isn't a word at all. Throws
// SyntaxError for a ${ with no }.
std::optional<std::string> expand_word(const std::string& word, const Shell& sh);

// A shell variable, or an environment variable; nullopt if neither.
std::optional<std::string> lookup_var(const Shell& sh, const std::string& name);
// Sets a variable: in the environment if it's exported, else in sh.vars.
void set_var(Shell& sh, const std::string& name, const std::string& value);
// Moves a shell variable into the environment (or makes an empty one).
void export_var(Shell& sh, const std::string& name);
// Removes a variable from both.
void unset_var(Shell& sh, const std::string& name);
bool is_valid_name(const std::string& name);

// Adds a line to the history, unless it's blank or the same as the last;
// keeps the newest 1000.
void add_history(Shell& sh, const std::string& line);
int builtin_history(Shell& sh, const std::vector<std::string>& argv);
// Reads a history file, through add_history. A missing file is no history.
void load_history(Shell& sh, const std::string& path);
// Writes the history to path, through a temporary file renamed over it,
// readable only by its owner.
void save_history(const Shell& sh, const std::string& path);

// Reads a line at the terminal, with editing and the history; nullopt at
// the end of input (Ctrl-D on an empty line).
std::optional<std::string> read_edited_line(Shell& sh, const std::string& prompt);
