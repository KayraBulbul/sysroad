// The course's version of this file. You won't need to change it.
#include <fcntl.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>

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
    if (p.background || p.commands.size() != 1) {
        return false;
    }
    const Command& c = p.commands[0];
    return c.argv.empty() || find_builtin(c.argv[0]) != nullptr;
}

int run_pipeline(Shell& sh, const Pipeline& p) {
    if (runs_in_shell(p)) {
        return run_in_shell(sh, p.commands[0]);
    }
    Job job = launch_job(sh, p);
    if (p.background) {
        return start_background(sh, std::move(job));
    }
    return run_foreground(sh, std::move(job));
}

Job launch_job(Shell& sh, const Pipeline& p) {
    Job job;
    job.text = p.text;
    bool control = sh.interactive;
    int prev_read = -1;
    for (size_t i = 0; i < p.commands.size(); ++i) {
        int fds[2] = {-1, -1};
        if (i + 1 < p.commands.size() && pipe(fds) == -1) {
            break;
        }
        std::fflush(nullptr);
        pid_t pid = fork();
        if (pid == 0) {
            if (control) {
                setpgid(0, job.pgid);
                if (!p.background) {
                    tcsetpgrp(sh.tty, getpgrp());
                }
            }
            for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU, SIGCHLD, SIGPIPE}) {
                signal(sig, SIG_DFL);
            }
            if (prev_read != -1) {
                dup2(prev_read, 0);
                close(prev_read);
            } else if (p.background && !control) {
                // Without job control, a background job mustn't read the
                // shell's input.
                int null = open("/dev/null", O_RDONLY);
                dup2(null, 0);
                close(null);
            }
            if (fds[1] != -1) {
                dup2(fds[1], 1);
                close(fds[1]);
                close(fds[0]);
            }
            sh.interactive = false;
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
            break;
        }
        if (control) {
            if (job.pgid == 0) {
                job.pgid = pid;
                if (!p.background) {
                    tcsetpgrp(sh.tty, job.pgid);
                }
            }
            setpgid(pid, job.pgid);
        }
        job.pids.push_back(pid);
        job.ended.push_back(false);
        job.statuses.emplace_back();
    }
    if (prev_read != -1) {
        close(prev_read);
    }
    return job;
}
