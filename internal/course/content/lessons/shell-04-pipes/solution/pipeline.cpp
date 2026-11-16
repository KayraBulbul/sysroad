#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

#include "conch.hpp"

int run_list(Shell& sh, const List& list) {
    Step::Next before = Step::Then;
    for (const Step& step : list) {
        bool run = before == Step::Then || (before == Step::And && sh.last_status == 0) ||
                   (before == Step::Or && sh.last_status != 0);
        if (run) {
            sh.last_status = run_pipeline(sh, step.pipeline);
        }
        if (sh.exiting) {
            break;
        }
        before = step.next;
    }
    return sh.last_status;
}

// Gives up on a pipeline that couldn't be started: ends the commands
// already running (they'd wait forever on a half-built pipeline) and
// throws, naming the call that failed.
[[noreturn]] static void abandon(const std::vector<pid_t>& pids, int prev_read, const char* what) {
    int err = errno;
    if (prev_read != -1) {
        close(prev_read);
    }
    for (pid_t pid : pids) {
        kill(pid, SIGKILL);
    }
    for (pid_t pid : pids) {
        waitpid(pid, nullptr, 0);
    }
    throw std::system_error(err, std::generic_category(), what);
}

// Starts each command in a child, stdout to the next one's stdin.
static std::vector<pid_t> start(Shell& sh, const Pipeline& p) {
    std::vector<pid_t> pids;
    int prev_read = -1;
    for (size_t i = 0; i < p.commands.size(); ++i) {
        int fds[2] = {-1, -1};
        if (i + 1 < p.commands.size() && pipe(fds) == -1) {
            abandon(pids, prev_read, "pipe");
        }
        std::fflush(nullptr);
        pid_t pid = fork();
        if (pid == 0) {
            if (prev_read != -1) {
                dup2(prev_read, 0);
                close(prev_read);
            }
            if (fds[1] != -1) {
                dup2(fds[1], 1);
                close(fds[1]);
                close(fds[0]);
            }
            exec_command(sh, p.commands[i]);
        }
        if (prev_read != -1) {
            close(prev_read);
        }
        if (fds[1] != -1) {
            close(fds[1]);
        }
        prev_read = fds[0];
        if (pid == -1) {
            abandon(pids, prev_read, "fork");
        }
        pids.push_back(pid);
    }
    return pids;
}

int run_pipeline(Shell& sh, const Pipeline& p) {
    if (p.commands.size() == 1) {
        const Command& c = p.commands[0];
        Builtin builtin = c.argv.empty() ? nullptr : find_builtin(c.argv[0]);
        if (builtin != nullptr) {
            return run_builtin(sh, builtin, c);
        }
    }
    std::vector<pid_t> pids;
    try {
        pids = start(sh, p);
    } catch (const std::system_error& e) {
        std::cerr << "conch: " << e.what() << '\n';
        return 1;
    }
    int status = 0;
    for (pid_t pid : pids) {
        int wstatus = 0;
        while (waitpid(pid, &wstatus, 0) == -1 && errno == EINTR) {
        }
        status = shell_status(wstatus);  // the last one's counts
    }
    return status;
}
