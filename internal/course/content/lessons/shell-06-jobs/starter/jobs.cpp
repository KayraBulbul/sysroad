#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <iostream>

#include "shell.hpp"

void init_job_control(Shell& sh) {
    (void)sh;  // TODO: ignore the job signals, lead a group, take the terminal
}

static int next_number(const Shell& sh) {
    int n = 0;
    for (const Job& j : sh.jobs) {
        n = std::max(n, j.number);
    }
    return n + 1;
}

// Waits for every process in the job that hasn't ended yet.
// TODO: with WUNTRACED, stopping when one stops.
static void wait_for_job(Job& job) {
    for (size_t i = 0; i < job.pids.size(); ++i) {
        if (!job.ended[i]) {
            job.statuses[i] = wait_for(job.pids[i]);
            job.ended[i] = true;
        }
    }
}

int start_background(Shell& sh, Job job) {
    if (job.pids.empty()) {
        return 1;  // nothing started
    }
    job.number = next_number(sh);
    std::cerr << '[' << job.number << "] " << job.pids.back() << '\n';
    sh.jobs.push_back(std::move(job));
    return 0;
}

int run_foreground(Shell&, Job job) {
    wait_for_job(job);
    return job.statuses.empty() ? 1 : shell_status(job.statuses.back());
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
            if (!job.ended[i] && waitpid(job.pids[i], &st, WNOHANG) == job.pids[i]) {
                job.ended[i] = true;
                job.statuses[i] = decode(st);
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

int builtin_fg(Shell& sh, const std::vector<std::string>& argv) {
    (void)sh;  // TODO
    (void)argv;
    std::cerr << "conch: fg: no job control\n";
    return 1;
}

int builtin_bg(Shell& sh, const std::vector<std::string>& argv) {
    (void)sh;  // TODO
    (void)argv;
    std::cerr << "conch: bg: no job control\n";
    return 1;
}

bool may_exit(Shell& sh) {
    (void)sh;  // TODO: warn once about stopped jobs
    return true;
}
