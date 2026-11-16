#include <sys/wait.h>
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

static int next_number(const Shell& sh) {
    int n = 0;
    for (const Job& j : sh.jobs) {
        n = std::max(n, j.number);
    }
    return n + 1;
}

// Waits for every process of the job that hasn't ended.
static void wait_for_job(Job& job) {
    for (size_t i = 0; i < job.pids.size(); ++i) {
        while (!job.ended[i]) {
            int st = 0;
            if (waitpid(job.pids[i], &st, 0) == -1) {
                if (errno == EINTR) {
                    continue;
                }
                st = 0;  // ECHILD: already reaped
            }
            job.ended[i] = true;
            job.statuses[i] = st;
        }
    }
}

int run_foreground(Shell&, Job job) {
    wait_for_job(job);
    return job.statuses.empty() ? 1 : shell_status(job.statuses.back());
}

int start_background(Shell& sh, Job job) {
    job.number = next_number(sh);
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
            if (!job.ended[i] && waitpid(job.pids[i], &st, WNOHANG) == job.pids[i]) {
                job.ended[i] = true;
                job.statuses[i] = st;
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
        std::cout << '[' << j.number << "] Running  " << j.text << '\n';
    }
    return 0;
}

int builtin_wait(Shell& sh, const std::vector<std::string>& argv) {
    if (argv.size() < 2) {
        for (Job& j : sh.jobs) {
            wait_for_job(j);
        }
        sh.jobs.clear();
        return 0;
    }
    std::string spec = argv[1][0] == '%' ? argv[1].substr(1) : argv[1];
    for (auto it = sh.jobs.begin(); it != sh.jobs.end(); ++it) {
        if (std::to_string(it->number) == spec) {
            wait_for_job(*it);
            int status = shell_status(it->statuses.back());
            sh.jobs.erase(it);
            return status;
        }
    }
    std::cerr << "conch: wait: " << argv[1] << ": no such job\n";
    return 127;
}
