#include "jobctl.hpp"

#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

// The signals a shell ignores, and its jobs must not.
static const int kJobSignals[] = {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU};

void init_job_control(int tty) {
    // In the background? Wait to be brought to the foreground rather than
    // steal the terminal: SIGTTIN stops us until then.
    for (pid_t fg = tcgetpgrp(tty); fg != -1 && fg != getpgrp(); fg = tcgetpgrp(tty)) {
        kill(-getpgrp(), SIGTTIN);
    }
    for (int sig : kJobSignals) {
        signal(sig, SIG_IGN);
    }
    if (getpgrp() != getpid()) {
        setpgid(0, 0);  // a session leader already leads its group
    }
    tcsetpgrp(tty, getpgrp());  // allowed in the background: SIGTTOU is ignored
}

static std::vector<char*> c_args(const Command& command) {
    std::vector<char*> args;
    for (const auto& a : command) {
        args.push_back(const_cast<char*>(a.c_str()));
    }
    args.push_back(nullptr);
    return args;
}

// Gives up on a job that couldn't be started: ends and reaps the
// commands already running (they'd wait forever on a half-built
// pipeline), then throws.
[[noreturn]] static void abandon(const Job& job, int prev_read, const char* what) {
    int err = errno;
    if (prev_read != -1) {
        close(prev_read);
    }
    for (pid_t pid : job.pids) {
        kill(pid, SIGKILL);
    }
    for (pid_t pid : job.pids) {
        waitpid(pid, nullptr, 0);
    }
    throw std::system_error(err, std::generic_category(), what);
}

Job launch_job(const std::vector<Command>& commands, bool foreground, int tty) {
    Job job;
    int prev_read = -1;
    for (size_t i = 0; i < commands.size(); ++i) {
        std::vector<char*> args = c_args(commands[i]);
        int p[2] = {-1, -1};
        if (i + 1 < commands.size() && pipe(p) == -1) {
            abandon(job, prev_read, "pipe");
        }
        std::fflush(nullptr);
        pid_t pid = fork();
        if (pid == 0) {
            setpgid(0, job.pgid);
            if (foreground) {
                tcsetpgrp(tty, getpgrp());
            }
            for (int sig : kJobSignals) {
                signal(sig, SIG_DFL);
            }
            signal(SIGPIPE, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            if (prev_read != -1) {
                dup2(prev_read, 0);
                close(prev_read);
            }
            if (p[1] != -1) {
                dup2(p[1], 1);
                close(p[1]);
                close(p[0]);
            }
            execvp(args[0], args.data());
            std::fprintf(stderr, "%s: %s\n", args[0], std::strerror(errno));
            _exit(127);
        }
        if (prev_read != -1) {
            close(prev_read);
        }
        if (p[1] != -1) {
            close(p[1]);
        }
        prev_read = p[0];
        if (pid == -1) {
            abandon(job, prev_read, "fork");
        }
        if (job.pgid == 0) {
            job.pgid = pid;
            if (foreground) {
                tcsetpgrp(tty, job.pgid);
            }
        }
        setpgid(pid, job.pgid);
        job.pids.push_back(pid);
        job.ended.push_back(false);
        job.statuses.emplace_back();
    }
    if (prev_read != -1) {
        close(prev_read);
    }
    return job;
}

void wait_for_job(Job& job) {
    job.stopped = false;
    while (!job.done()) {
        int st = 0;
        pid_t pid = waitpid(-job.pgid, &st, WUNTRACED);
        if (pid == -1) {
            if (errno == EINTR) {
                continue;
            }
            break;  // ECHILD: someone else reaped them
        }
        for (size_t i = 0; i < job.pids.size(); ++i) {
            if (job.pids[i] != pid) {
                continue;
            }
            if (WIFSTOPPED(st)) {
                job.stopped = true;
                job.stop_signal = WSTOPSIG(st);
            } else {
                job.ended[i] = true;
                job.statuses[i] = decode(st);
            }
        }
        if (job.stopped) {
            return;
        }
    }
}

// Waits for the job while it holds the terminal, after a SIGCONT if asked.
static void in_foreground(Job& job, int tty, bool resume) {
    termios shell_modes{};
    bool have_modes = tcgetattr(tty, &shell_modes) == 0;
    tcsetpgrp(tty, job.pgid);
    if (resume) {
        if (job.has_modes) {
            tcsetattr(tty, TCSADRAIN, &job.modes);  // as it was when it stopped
        }
        kill(-job.pgid, SIGCONT);
    }
    wait_for_job(job);
    if (job.stopped) {
        job.has_modes = tcgetattr(tty, &job.modes) == 0;  // keep them for later
    }
    tcsetpgrp(tty, getpgrp());
    if (have_modes) {
        tcsetattr(tty, TCSADRAIN, &shell_modes);  // the job may have left raw mode on
    }
}

void wait_foreground(Job& job, int tty) {
    in_foreground(job, tty, false);
}

void continue_job(Job& job, int tty, bool foreground) {
    job.stopped = false;
    if (foreground) {
        in_foreground(job, tty, true);
    } else {
        kill(-job.pgid, SIGCONT);
    }
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
