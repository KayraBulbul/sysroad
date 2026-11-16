#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <iostream>

#include "conch.hpp"

std::string signal_name(int sig) {
    switch (sig) {
    case SIGHUP: return "SIGHUP";
    case SIGINT: return "SIGINT";
    case SIGQUIT: return "SIGQUIT";
    case SIGABRT: return "SIGABRT";
    case SIGKILL: return "SIGKILL";
    case SIGSEGV: return "SIGSEGV";
    case SIGPIPE: return "SIGPIPE";
    case SIGALRM: return "SIGALRM";
    case SIGTERM: return "SIGTERM";
    case SIGUSR1: return "SIGUSR1";
    case SIGUSR2: return "SIGUSR2";
    case SIGTSTP: return "SIGTSTP";
    case SIGSTOP: return "SIGSTOP";
    case SIGTTIN: return "SIGTTIN";
    case SIGTTOU: return "SIGTTOU";
    default: return "signal " + std::to_string(sig);
    }
}

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
            job.statuses[i] = st;
        }
    }
}

// Waits until every process of the job has ended, or one has stopped.
static void wait_for_job(Job& job) {
    job.stopped = false;
    for (size_t i = 0; i < job.pids.size() && !job.stopped; ++i) {
        while (!job.ended[i] && !job.stopped) {
            int st = 0;
            if (waitpid(job.pids[i], &st, WUNTRACED) == -1) {
                if (errno == EINTR) {
                    continue;
                }
                job.ended[i] = true;  // ECHILD: already reaped
                break;
            }
            update(job, job.pids[i], st);
        }
    }
}

// The foreground: hand over the terminal, wait, take the terminal back
// with the shell's own modes.
static void in_foreground(Shell& sh, Job& job, bool resume) {
    termios modes{};
    bool have_modes = sh.interactive && tcgetattr(sh.tty, &modes) == 0;
    if (sh.interactive) {
        tcsetpgrp(sh.tty, job.pgid);
    }
    if (resume) {
        if (sh.interactive && job.has_modes) {
            tcsetattr(sh.tty, TCSADRAIN, &job.modes);  // as it was when it stopped
        }
        kill(-job.pgid, SIGCONT);
    }
    wait_for_job(job);
    if (sh.interactive) {
        if (job.stopped) {
            // vim in raw mode, say: keep its settings, then put ours back.
            job.has_modes = tcgetattr(sh.tty, &job.modes) == 0;
        }
        tcsetpgrp(sh.tty, getpgrp());
    }
    if (have_modes) {
        tcsetattr(sh.tty, TCSADRAIN, &modes);
    }
}

// A job that stopped goes in the table (back under its number, if it was
// there), and $? is 128 plus the signal that stopped it.
static int stopped_status(Shell& sh, Job job) {
    if (job.number == 0) {
        job.number = next_number(sh);
    }
    std::cout << "\n[" << job.number << "] Stopped  " << job.text << std::endl;
    int status = 128 + job.stop_signal;
    for (Job& j : sh.jobs) {
        if (j.number == job.number) {
            j = job;
            return status;
        }
    }
    sh.jobs.push_back(std::move(job));
    return status;
}

// $? for a job that has ended.
static int ended_status(const Shell& sh, const Job& job) {
    int last = job.statuses.empty() ? 0 : job.statuses.back();
    if (sh.interactive && WIFSIGNALED(last) && WTERMSIG(last) == SIGINT) {
        std::cout << std::endl;  // after the ^C, a fresh line for the prompt
    }
    return job.statuses.empty() ? 1 : shell_status(last);
}

int run_foreground(Shell& sh, Job job) {
    in_foreground(sh, job, false);
    if (job.stopped) {
        return stopped_status(sh, std::move(job));
    }
    return ended_status(sh, job);
}

int start_background(Shell& sh, Job job) {
    job.number = next_number(sh);
    sh.last_background = job.pids.back();
    std::cerr << '[' << job.number << "] " << job.pids.back() << '\n';
    sh.jobs.push_back(std::move(job));
    return 0;
}

// How a finished job ended, for its notice.
static std::string ended_text(const Job& job) {
    int st = job.statuses.back();
    if (WIFSIGNALED(st)) {
        return signal_name(WTERMSIG(st));
    }
    if (WEXITSTATUS(st) == 0) {
        return "Done";
    }
    return "Exit " + std::to_string(WEXITSTATUS(st));
}

void notify_jobs(Shell& sh) {
    for (Job& job : sh.jobs) {
        for (size_t i = 0; i < job.pids.size(); ++i) {
            int st = 0;
            if (!job.ended[i] && waitpid(job.pids[i], &st, WNOHANG | WUNTRACED | WCONTINUED) == job.pids[i]) {
                update(job, job.pids[i], st);
            }
        }
    }
    for (auto it = sh.jobs.begin(); it != sh.jobs.end();) {
        if (it->done()) {
            std::cout << '[' << it->number << "] " << ended_text(*it) << "  " << it->text << std::endl;
            it = sh.jobs.erase(it);
        } else {
            ++it;
        }
    }
}

int builtin_jobs(Shell& sh, const std::vector<std::string>&) {
    notify_jobs(sh);
    for (const Job& j : sh.jobs) {
        std::cout << '[' << j.number << "] " << (j.stopped ? "Stopped" : "Running") << "  " << j.text << '\n';
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
    std::string spec = argv[1][0] == '%' ? argv[1].substr(1) : argv[1];
    for (auto it = sh.jobs.begin(); it != sh.jobs.end(); ++it) {
        if (std::to_string(it->number) != spec) {
            continue;
        }
        if (!it->stopped) {
            wait_for_job(*it);
        }
        if (it->stopped) {
            return 128 + it->stop_signal;  // it won't end until it's continued
        }
        int status = shell_status(it->statuses.back());
        sh.jobs.erase(it);
        return status;
    }
    std::cerr << "conch: wait: " << argv[1] << ": no such job\n";
    return 127;
}

// The job a %n names; with no argument, the newest one.
static Job* find_job(Shell& sh, const std::vector<std::string>& argv, const char* who) {
    if (sh.jobs.empty()) {
        std::cerr << "conch: " << who << ": no current job\n";
        return nullptr;
    }
    if (argv.size() < 2) {
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
