#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>

#include "shell.hpp"

int start_background(Shell& sh, Job job) {
    // TODO: number it, print "[n] pid" to stderr, and keep it in sh.jobs.
    // For now it's waited for, as if it were in the foreground.
    return run_foreground(sh, std::move(job));
}

int run_foreground(Shell&, Job job) {
    int status = 1;
    for (pid_t pid : job.pids) {
        status = shell_status(wait_for(pid));
    }
    return status;
}

void notify_jobs(Shell& sh) {
    (void)sh;  // TODO: notice jobs that have ended, without waiting
}

int builtin_jobs(Shell& sh, const std::vector<std::string>& argv) {
    (void)sh;  // TODO
    (void)argv;
    return 0;
}

int builtin_wait(Shell& sh, const std::vector<std::string>& argv) {
    (void)sh;  // TODO
    (void)argv;
    return 0;
}
