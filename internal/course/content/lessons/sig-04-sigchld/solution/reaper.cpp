#include "reaper.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

// What the handler has reaped and collect() hasn't taken yet. The handler
// may only use plain arrays and sig_atomic_t, not std::vector.
static const int kMax = 4096;
static pid_t reaped_pid[kMax];
static int reaped_status[kMax];
static volatile sig_atomic_t reaped_count = 0;

static void on_sigchld(int) {
    int saved = errno;
    // One SIGCHLD can stand for many children: signals don't queue. Stop
    // when the arrays are full, leaving the rest unreaped for collect().
    while (reaped_count < kMax) {
        int status = 0;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) {
            break;
        }
        reaped_pid[reaped_count] = pid;
        reaped_status[reaped_count] = status;
        reaped_count = reaped_count + 1;
    }
    errno = saved;
}

static sigset_t only_sigchld() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    return set;
}

Reaper::Reaper() {
    struct sigaction sa{};
    sa.sa_handler = on_sigchld;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, &old_);
}

Reaper::~Reaper() {
    sigaction(SIGCHLD, &old_, nullptr);
}

void Reaper::collect() {
    // If the handler's arrays filled up, some children are still waiting
    // to be reaped: reap them here (SIGCHLD is blocked, so the handler
    // can't run meanwhile), until a pass finds the arrays not full.
    bool full = true;
    while (full) {
        full = reaped_count == kMax;
        for (int i = 0; i < reaped_count; ++i) {
            running_.erase(reaped_pid[i]);
            done_.emplace_back(reaped_pid[i], decode(reaped_status[i]));
        }
        reaped_count = 0;
        if (full) {
            on_sigchld(SIGCHLD);
        }
    }
}

pid_t Reaper::start(const std::vector<std::string>& argv) {
    std::vector<char*> args;
    for (const auto& a : argv) {
        args.push_back(const_cast<char*>(a.c_str()));
    }
    args.push_back(nullptr);
    sigset_t block = only_sigchld(), old;
    sigprocmask(SIG_BLOCK, &block, &old);
    std::fflush(nullptr);
    pid_t pid = fork();
    if (pid == 0) {
        sigprocmask(SIG_SETMASK, &old, nullptr);
        execvp(args[0], args.data());
        std::fprintf(stderr, "%s: %s\n", args[0], std::strerror(errno));
        _exit(127);
    }
    if (pid != -1) {
        running_.insert(pid);  // before the handler can see it end
    }
    int err = errno;
    sigprocmask(SIG_SETMASK, &old, nullptr);
    if (pid == -1) {
        throw std::system_error(err, std::generic_category(), "fork");
    }
    return pid;
}

std::vector<std::pair<pid_t, Status>> Reaper::finished() {
    sigset_t block = only_sigchld(), old;
    sigprocmask(SIG_BLOCK, &block, &old);
    collect();
    std::vector<std::pair<pid_t, Status>> out;
    out.swap(done_);
    sigprocmask(SIG_SETMASK, &old, nullptr);
    return out;
}

size_t Reaper::running() {
    sigset_t block = only_sigchld(), old;
    sigprocmask(SIG_BLOCK, &block, &old);
    collect();
    size_t n = running_.size();
    sigprocmask(SIG_SETMASK, &old, nullptr);
    return n;
}

void Reaper::wait_all() {
    sigset_t block = only_sigchld(), old;
    sigprocmask(SIG_BLOCK, &block, &old);
    sigset_t during = old;
    sigdelset(&during, SIGCHLD);
    collect();
    while (!running_.empty()) {
        sigsuspend(&during);
        collect();
    }
    sigprocmask(SIG_SETMASK, &old, nullptr);
}

// reaper: starts five sleeps of different lengths, all at once, and
// reports each as it ends.
int main() {
    Reaper reaper;
    for (const char* secs : {"0.5", "0.1", "0.4", "0.2", "0.3"}) {
        pid_t pid = reaper.start({"sleep", secs});
        std::cout << "started sleep " << secs << " as " << pid << '\n';
    }
    std::cout << reaper.running() << " running\n";
    while (reaper.running() > 0) {
        usleep(50'000);  // the handler reaps; we just look now and then
        for (const auto& [pid, status] : reaper.finished()) {
            std::cout << pid << " " << describe(status) << '\n';
        }
    }
    std::cout << "all done, no zombies\n";
    return 0;
}
