#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <sys/types.h>

#include <string>
#include <vector>

#include "proc.hpp"

using Command = std::vector<std::string>;

// A pipeline running in a process group of its own.
struct Job {
    pid_t pgid = 0;           // the group's ID: the first command's PID
    std::vector<pid_t> pids;  // one per command, in order
};

// Starts commands as a pipeline, like last chapter's run_pipeline, but
// in a new process group led by the first command, and without waiting.
// Every process is in the group before its command runs, and by the time
// launch returns.
Job launch(const std::vector<Command>& commands);

// Sends sig to every process in the job at once. Throws std::system_error
// if that fails (say, they've all been reaped).
void signal_job(const Job& job, int sig);

// Waits for every process in the job, and returns how each ended, in order.
std::vector<Status> wait_job(const Job& job);

// Starts argv in a new session of its own, with no controlling terminal.
pid_t start_detached(const Command& argv);
