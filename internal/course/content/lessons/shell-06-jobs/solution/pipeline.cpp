#include <fcntl.h>
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
[[noreturn]] static void abandon(Shell& sh, const std::vector<pid_t>& pids, int prev_read, const char* what) {
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
    if (sh.interactive) {
        tcsetpgrp(sh.tty, getpgrp());  // the terminal is the shell's again
    }
    throw std::system_error(err, std::generic_category(), what);
}

Job launch_job(Shell& sh, const Pipeline& p) {
    Job job;
    job.text = p.text;
    bool control = sh.interactive;
    std::vector<pid_t>& pids = job.pids;
    int prev_read = -1;
    for (size_t i = 0; i < p.commands.size(); ++i) {
        int fds[2] = {-1, -1};
        if (i + 1 < p.commands.size() && pipe(fds) == -1) {
            abandon(sh, pids, prev_read, "pipe");
        }
        std::fflush(nullptr);
        pid_t pid = fork();
        if (pid == 0) {
            if (control) {
                setpgid(0, job.pgid);  // 0 for the first: a group of its own
                if (!p.background) {
                    tcsetpgrp(sh.tty, getpgrp());
                }
            }
            for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU}) {
                signal(sig, SIG_DFL);
            }
            sh.interactive = false;  // a builtin in here has no job control
            if (prev_read != -1) {
                dup2(prev_read, 0);
                close(prev_read);
            } else if (p.background && !control) {
                // Without job control, a background job mustn't read the
                // shell's input. With it, reading the terminal stops it.
                int null = open("/dev/null", O_RDONLY);
                dup2(null, 0);
                close(null);
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
            abandon(sh, pids, prev_read, "fork");
        }
        if (control) {
            // Both sides set the group, so neither has to wait for the other.
            if (job.pgid == 0) {
                job.pgid = pid;
                if (!p.background) {
                    tcsetpgrp(sh.tty, job.pgid);
                }
            }
            setpgid(pid, job.pgid);
        }
        pids.push_back(pid);
        job.ended.push_back(false);
        job.statuses.push_back(0);
    }
    return job;
}

int run_pipeline(Shell& sh, const Pipeline& p) {
    if (p.commands.size() == 1 && !p.background) {
        const Command& c = p.commands[0];
        Builtin builtin = c.argv.empty() ? nullptr : find_builtin(c.argv[0]);
        if (builtin != nullptr) {
            return run_builtin(sh, builtin, c);
        }
    }
    Job job;
    try {
        job = launch_job(sh, p);
    } catch (const std::system_error& e) {
        std::cerr << "conch: " << e.what() << '\n';
        return 1;
    }
    if (p.background) {
        return start_background(sh, std::move(job));
    }
    return run_foreground(sh, std::move(job));
}
