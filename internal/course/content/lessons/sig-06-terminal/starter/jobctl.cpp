#include "jobctl.hpp"

#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>

void init_job_control(int tty) {
    (void)tty;  // TODO
}

Job launch_job(const std::vector<Command>& commands, bool foreground, int tty) {
    // TODO: start from 6.2.5's launch. In the child: setpgid, tcsetpgrp
    // if it's in the foreground, then the default signal actions.
    (void)foreground;
    (void)tty;
    Job job;
    for (const Command& c : commands) {
        job.pids.push_back(start(c));
        job.ended.push_back(false);
        job.statuses.emplace_back();
    }
    return job;
}

void wait_for_job(Job& job) {
    // TODO: waitpid(-job.pgid, ..., WUNTRACED) until done or stopped.
    for (size_t i = 0; i < job.pids.size(); ++i) {
        job.statuses[i] = wait_for(job.pids[i]);
        job.ended[i] = true;
    }
}

void wait_foreground(Job& job, int tty) {
    (void)tty;  // TODO: hand over the terminal, and take it back
    wait_for_job(job);
}

void continue_job(Job& job, int tty, bool foreground) {
    (void)job;  // TODO
    (void)tty;
    (void)foreground;
}

// jobrun COMMAND [ARG]...: runs COMMAND as a foreground job. Press Ctrl-Z
// and it stops; jobrun carries it on two seconds later. Ctrl-C ends it.
int main(int argc, char* argv[]) {
    if (argc < 2 || !isatty(0)) {
        std::cerr << "usage: jobrun COMMAND [ARG]..., on a terminal\n";
        return 2;
    }
    init_job_control(0);
    Job job = launch_job({Command(argv + 1, argv + argc)}, true, 0);
    wait_foreground(job, 0);
    while (job.stopped) {
        std::cout << "\njobrun: stopped by " << signal_name(job.stop_signal)
                  << "; carrying on in 2 seconds" << std::endl;
        sleep(2);
        continue_job(job, 0, true);
    }
    if (job.done()) {
        std::cout << "jobrun: " << describe(job.statuses.back()) << '\n';
    }
    return 0;
}
