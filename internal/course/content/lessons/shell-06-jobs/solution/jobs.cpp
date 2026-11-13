#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <iostream>

#include "shell.hpp"

void init_job_control(Shell& sh) {
    // Started in the background (conch &)? Wait to be brought to the
    // foreground: SIGTTIN stops us until then. Taking the terminal now
    // would steal it from the shell that started us.
    for (pid_t fg = tcgetpgrp(sh.tty); fg != -1 && fg != getpgrp(); fg = tcgetpgrp(sh.tty)) {
        kill(-getpgrp(), SIGTTIN);
    }
    for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU}) {
        signal(sig, SIG_IGN);
    }
    if (getpgrp() != getpid()) {
        setpgid(0, 0);
    }
    tcsetpgrp(sh.tty, getpgrp());
}

static int next_number(const Shell& sh) {
    int n = 0;
    for (const Job& j : sh.jobs) {
        n = std::max(n, j.number);
    }
    return n + 1;
}

// Records what waitpid said about pid.
static void update(Job& job, pid_t pid, int st) {
    for (size_t i = 0; i < job.pids.size(); ++i) {
        if (job.pids[i] != pid) {
            continue;
        }
        if (WIFSTOPPED(st)) {
            job.stopped = true;
            job.stop_signal = WSTOPSIG(st);
        } else if (WIFCONTINUED(st)) {
            job.stopped = false;
        } else {
            job.ended[i] = true;
            job.statuses[i] = decode(st);
        }
    }
}

// Waits until every process has ended or one has stopped.
static void wait_for_job(Job& job) {
    job.stopped = false;
    for (size_t i = 0; i < job.pids.size() && !job.stopped; ++i) {
        while (!job.ended[i] && !job.stopped) {
            int st = 0;
            pid_t pid = waitpid(job.pids[i], &st, WUNTRACED);
            if (pid == -1) {
                if (errno == EINTR) {
                    continue;
                }
                job.ended[i] = true;  // ECHILD: already reaped
                break;
            }
            update(job, pid, st);
        }
    }
}

// The foreground dance: hand over the terminal, optionally SIGCONT, wait,
// take the terminal back with the shell's own modes.
static void in_foreground(Shell& sh, Job& job, bool resume) {
    termios modes{};
    bool have_modes = sh.interactive && tcgetattr(sh.tty, &modes) == 0;
    if (sh.interactive) {
        tcsetpgrp(sh.tty, job.pgid);
    }
    if (resume) {
        kill(-job.pgid, SIGCONT);
    }
    wait_for_job(job);
    if (sh.interactive) {
        tcsetpgrp(sh.tty, getpgrp());
    }
    if (have_modes) {
        tcsetattr(sh.tty, TCSADRAIN, &modes);
    }
}

int start_background(Shell& sh, Job job) {
    if (job.pids.empty()) {
        return 1;  // nothing started
    }
    job.number = next_number(sh);

    std::cerr << '[' << job.number << "] " << (job.pgid != 0 ? job.pgid : job.pids.back()) << '\n';
    sh.jobs.push_back(std::move(job));
    return 0;
}

static int stopped_status(Shell& sh, Job job) {
    if (job.number == 0) {
        job.number = next_number(sh);
    }
    std::cout << "\n[" << job.number << "] Stopped  " << job.text << std::endl;
    int status = 128 + job.stop_signal;
    bool known = false;
    for (Job& j : sh.jobs) {
        if (j.number == job.number) {
            j = job;
            known = true;
        }
    }
    if (!known) {
        sh.jobs.push_back(std::move(job));
    }
    return status;
}

// $? for a job that ended. A Ctrl-C leaves the cursor after "^C", so move
// to a fresh line for the prompt.
static int ended_status(const Shell& sh, const Job& job) {
    if (job.statuses.empty()) {
        return 1;
    }
    const Status& last = job.statuses.back();
    if (sh.interactive && last.kind == Status::Signaled && last.code == SIGINT) {
        std::cout << std::endl;
    }
    return shell_status(last);
}

int run_foreground(Shell& sh, Job job) {
    in_foreground(sh, job, false);
    if (job.stopped) {
        return stopped_status(sh, std::move(job));
    }
    return ended_status(sh, job);
}

static std::string ended_text(const Job& job) {
    const Status& s = job.statuses.back();
    if (s.ok()) {
        return "Done";
    }
    if (s.kind == Status::Exited) {
        return "Exit " + std::to_string(s.code);
    }
    return signal_name(s.code);
}

void notify_jobs(Shell& sh) {
    for (Job& job : sh.jobs) {
        for (size_t i = 0; i < job.pids.size(); ++i) {
            int st = 0;
            if (!job.ended[i] &&
                waitpid(job.pids[i], &st, WNOHANG | WUNTRACED | WCONTINUED) == job.pids[i]) {
                update(job, job.pids[i], st);
            }
        }
    }
    for (auto it = sh.jobs.begin(); it != sh.jobs.end();) {
        if (it->done()) {
            std::cout << '[' << it->number << "] " << ended_text(*it) << "  " << it->text
                      << std::endl;
            it = sh.jobs.erase(it);
        } else {
            ++it;
        }
    }
}

// The job a %n names; with no argument, the newest one.
static Job* find_job(Shell& sh, const std::vector<std::string>& argv, const char* who) {
    if (sh.jobs.empty()) {
        std::cerr << "conch: " << who << ": no current job\n";
        return nullptr;
    }
    if (argv.size() < 2 || argv[1] == "%%" || argv[1] == "%+") {
        return &sh.jobs.back();
    }
    std::string spec = argv[1][0] == '%' ? argv[1].substr(1) : argv[1];
    for (Job& j : sh.jobs) {
        if (std::to_string(j.number) == spec) {
            return &j;
        }
    }
    std::cerr << "conch: " << who << ": " << argv[1] << ": no such job\n";
    return nullptr;
}

int builtin_jobs(Shell& sh, const std::vector<std::string>&) {
    notify_jobs(sh);
    for (const Job& j : sh.jobs) {
        std::cout << '[' << j.number << "] " << (j.stopped ? "Stopped" : "Running") << "  "
                  << j.text << '\n';
    }
    return 0;
}

int builtin_wait(Shell& sh, const std::vector<std::string>& argv) {
    if (argv.size() < 2) {
        for (Job& j : sh.jobs) {
            if (!j.stopped) {
                wait_for_job(j);
            }
        }
        for (auto it = sh.jobs.begin(); it != sh.jobs.end();) {
            it = it->done() ? sh.jobs.erase(it) : it + 1;
        }
        return 0;
    }
    Job* job = find_job(sh, argv, "wait");
    if (job == nullptr) {
        return 127;
    }
    wait_for_job(*job);
    if (job->stopped) {
        return 128 + job->stop_signal;
    }
    int status = shell_status(job->statuses.back());
    sh.jobs.erase(sh.jobs.begin() + (job - sh.jobs.data()));
    return status;
}

int builtin_fg(Shell& sh, const std::vector<std::string>& argv) {
    if (!sh.interactive) {
        std::cerr << "conch: fg: no job control\n";
        return 1;
    }
    Job* found = find_job(sh, argv, "fg");
    if (found == nullptr) {
        return 1;
    }
    Job job = *found;
    sh.jobs.erase(sh.jobs.begin() + (found - sh.jobs.data()));
    std::cout << job.text << std::endl;
    in_foreground(sh, job, true);
    if (job.stopped) {
        return stopped_status(sh, std::move(job));
    }
    return ended_status(sh, job);
}

int builtin_bg(Shell& sh, const std::vector<std::string>& argv) {
    if (!sh.interactive) {
        std::cerr << "conch: bg: no job control\n";
        return 1;
    }
    Job* job = find_job(sh, argv, "bg");
    if (job == nullptr) {
        return 1;
    }
    job->stopped = false;
    kill(-job->pgid, SIGCONT);
    std::cout << '[' << job->number << "] " << job->text << " &\n";
    return 0;
}

bool may_exit(Shell& sh) {
    bool stopped = false;
    for (const Job& j : sh.jobs) {
        stopped = stopped || j.stopped;
    }
    if (stopped && !sh.warned_stopped) {
        std::cerr << "conch: there are stopped jobs\n";
        sh.warned_stopped = true;
        return false;
    }
    for (const Job& j : sh.jobs) {
        if (j.stopped) {
            kill(-j.pgid, SIGHUP);
            kill(-j.pgid, SIGCONT);
        }
    }
    return true;
}
