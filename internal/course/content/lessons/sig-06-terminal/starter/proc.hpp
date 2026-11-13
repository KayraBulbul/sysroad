#pragma once
// The course's version of lesson 6.1.4. You won't need to change this file.

#include <sys/types.h>

#include <string>
#include <utility>
#include <vector>

// How a child ended.
struct Status {
    enum Kind { Exited, Signaled } kind = Exited;
    int code = 0;  // the exit code, or the signal's number
    bool core_dumped = false;

    bool ok() const {
        return kind == Exited && code == 0;
    }
};

// Forks and execs argv (searching PATH). If the exec fails, the child
// prints why to stderr and exits with 127. Returns the child's PID.
pid_t start(const std::vector<std::string>& argv);

// Turns a status from waitpid into a Status.
Status decode(int wstatus);

// Waits for one child, retrying when interrupted. Throws
// std::system_error if waitpid fails.
Status wait_for(pid_t pid);

// "SIGTERM" for SIGTERM, and so on for the common signals; "signal 42"
// for the rest.
std::string signal_name(int sig);

// "exited 0", "exited 3", "killed by SIGTERM", "killed by SIGSEGV (core dumped)".
std::string describe(const Status& s);

// The number a shell puts in $?: the exit code, or 128 + the signal.
int shell_status(const Status& s);

// Collects every child that has already finished, without waiting for
// the rest. Empty if none have finished, or there are no children.
std::vector<std::pair<pid_t, Status>> reap_finished();
