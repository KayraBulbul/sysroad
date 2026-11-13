#include "job.hpp"

#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

Job launch(const std::vector<Command>& commands) {
    // TODO: your pipeline from 6.1.7, plus setpgid in the child and in
    // the parent, and no waiting.
    (void)commands;
    return {};
}

void signal_job(const Job& job, int sig) {
    (void)job;  // TODO: kill with a negative PID
    (void)sig;
}

std::vector<Status> wait_job(const Job& job) {
    (void)job;  // TODO
    return {};
}

pid_t start_detached(const Command& argv) {
    return start(argv);  // TODO: setsid in the child
}

static void show(const char* who, pid_t pid) {
    std::cout << who << ": pid " << pid << ", group " << getpgid(pid) << ", session " << getsid(pid)
              << '\n';
}

// pgroup: starts a three-command job in a group of its own, shows the
// IDs, then stops the whole job with one kill.
int main() {
    show("pgroup  ", getpid());
    Job job = launch({{"sleep", "5"}, {"sleep", "5"}, {"sleep", "5"}});
    for (pid_t pid : job.pids) {
        show("job     ", pid);
    }
    pid_t loner = start_detached({"sleep", "0.2"});
    usleep(50'000);  // the child calls setsid; give it a moment to run
    show("detached", loner);

    signal_job(job, SIGTERM);
    std::cout << "sent SIGTERM to group " << job.pgid << ":";
    for (const Status& s : wait_job(job)) {
        std::cout << ' ' << describe(s) << ';';
    }
    std::cout << '\n';
    std::cout << "detached one " << describe(wait_for(loner)) << '\n';
    return 0;
}
