#include "job.hpp"

#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

static std::vector<char*> c_args(const Command& command) {
    std::vector<char*> args;
    for (const auto& a : command) {
        args.push_back(const_cast<char*>(a.c_str()));
    }
    args.push_back(nullptr);
    return args;
}

[[noreturn]] static void exec_or_die(std::vector<char*>& args) {
    signal(SIGPIPE, SIG_DFL);
    execvp(args[0], args.data());
    std::fprintf(stderr, "%s: %s\n", args[0], std::strerror(errno));
    _exit(127);
}

Job launch(const std::vector<Command>& commands) {
    Job job;
    int prev_read = -1;
    for (size_t i = 0; i < commands.size(); ++i) {
        std::vector<char*> args = c_args(commands[i]);
        int p[2] = {-1, -1};
        if (i + 1 < commands.size() && pipe(p) == -1) {
            break;
        }
        std::fflush(nullptr);
        pid_t pid = fork();
        if (pid == 0) {
            // Both sides call setpgid: whichever runs first wins the race,
            // and the other's call is harmless.
            setpgid(0, job.pgid);  // 0 for the first: a new group, our own PID
            if (prev_read != -1) {
                dup2(prev_read, 0);
                close(prev_read);
            }
            if (p[1] != -1) {
                dup2(p[1], 1);
                close(p[1]);
                close(p[0]);
            }
            exec_or_die(args);
        }
        if (prev_read != -1) {
            close(prev_read);
        }
        if (p[1] != -1) {
            close(p[1]);
        }
        prev_read = p[0];
        if (pid == -1) {
            break;
        }
        if (job.pgid == 0) {
            job.pgid = pid;
        }
        setpgid(pid, job.pgid);
        job.pids.push_back(pid);
    }
    if (prev_read != -1) {
        close(prev_read);
    }
    return job;
}

void signal_job(const Job& job, int sig) {
    if (kill(-job.pgid, sig) == -1) {
        throw std::system_error(errno, std::generic_category(), "kill");
    }
}

std::vector<Status> wait_job(const Job& job) {
    std::vector<Status> statuses;
    for (pid_t pid : job.pids) {
        statuses.push_back(wait_for(pid));
    }
    return statuses;
}

pid_t start_detached(const Command& argv) {
    std::vector<char*> args = c_args(argv);
    std::fflush(nullptr);
    pid_t pid = fork();
    if (pid == -1) {
        throw std::system_error(errno, std::generic_category(), "fork");
    }
    if (pid == 0) {
        setsid();
        exec_or_die(args);
    }
    return pid;
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
