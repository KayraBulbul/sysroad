#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <sys/types.h>
#include <termios.h>

#include <string>
#include <vector>

#include "proc.hpp"

using Command = std::vector<std::string>;

// A pipeline in a process group of its own, and what's known about it.
struct Job {
    pid_t pgid = 0;
    std::vector<pid_t> pids;       // one per command, in order
    std::vector<bool> ended;       // which processes have ended...
    std::vector<Status> statuses;  // ...and how
    bool stopped = false;
    int stop_signal = 0;  // what stopped it: SIGTSTP, SIGTTIN, ...
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

// Gets this process ready to run jobs on the terminal tty, as a shell
// does: it leads a process group, that group is the terminal's foreground
// group, and SIGINT, SIGQUIT, SIGTSTP, SIGTTIN and SIGTTOU are ignored.
void init_job_control(int tty);

// Starts commands as a pipeline in a new process group, like 6.2.5's
// launch. A foreground job is given the terminal. Every command starts
// with default actions for the signals init_job_control ignored. Throws
// std::system_error as launch does if a pipe or fork fails.
Job launch_job(const std::vector<Command>& commands, bool foreground, int tty);

// Waits until every process in the job has ended, or one has stopped,
// and records it in the job.
void wait_for_job(Job& job);

// Gives the job the terminal, waits for it as wait_for_job does, then
// takes the terminal back, with this process's own terminal modes. A job
// that stopped keeps the modes it left in job.modes.
void wait_foreground(Job& job, int tty);

// Lets a stopped job carry on (SIGCONT). In the foreground, it gets the
// terminal, with the modes it had when it stopped, and is waited for as
// in wait_foreground.
void continue_job(Job& job, int tty, bool foreground);
