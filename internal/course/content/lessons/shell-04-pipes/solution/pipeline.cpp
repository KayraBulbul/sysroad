#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "shell.hpp"

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

static bool runs_in_shell(const Pipeline& p) {
    if (p.commands.size() != 1) {
        return false;
    }
    const Command& c = p.commands[0];
    return c.argv.empty() || find_builtin(c.argv[0]) != nullptr;
}

// Gives up on a pipeline that couldn't be started: ends the commands
// already running (they'd wait forever on a half-built pipeline), says
// why, and returns the status for it.
static int abandon(const std::vector<pid_t>& pids, int prev_read, const char* what) {
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
    std::cerr << "conch: " << what << ": " << std::strerror(err) << '\n';
    return 1;
}

int run_pipeline(Shell& sh, const Pipeline& p) {
    if (runs_in_shell(p)) {
        return run_in_shell(sh, p.commands[0]);
    }
    std::vector<pid_t> pids;
    int prev_read = -1;
    for (size_t i = 0; i < p.commands.size(); ++i) {
        int fds[2] = {-1, -1};
        if (i + 1 < p.commands.size() && pipe(fds) == -1) {
            return abandon(pids, prev_read, "pipe");
        }
        std::fflush(nullptr);
        pid_t pid = fork();
        if (pid == 0) {
            signal(SIGPIPE, SIG_DFL);
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
            return abandon(pids, prev_read, "fork");
        }
        pids.push_back(pid);
    }
    if (prev_read != -1) {
        close(prev_read);
    }
    int status = 1;
    for (pid_t pid : pids) {
        status = shell_status(wait_for(pid));
    }
    return status;
}
