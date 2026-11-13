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
    // TODO: save errno; waitpid(-1, ..., WNOHANG) until it returns 0 or
    // -1, or the arrays are full, storing each one; restore errno.
}

Reaper::Reaper() {
    // TODO
    (void)on_sigchld;
    (void)old_;
}

Reaper::~Reaper() {
    // TODO
}

void Reaper::collect() {
    // TODO: move reaped_* into done_, erase them from running_, reset the
    // count. If the arrays were full, reap the rest yourself.
    (void)reaped_pid;
    (void)reaped_status;
    (void)reaped_count;
}

pid_t Reaper::start(const std::vector<std::string>& argv) {
    // TODO: block SIGCHLD first, so the child can't be reaped before it's
    // in running_. The child puts the old mask back before exec.
    return ::start(argv);
}

std::vector<std::pair<pid_t, Status>> Reaper::finished() {
    return {};  // TODO: block SIGCHLD, collect, hand done_ over
}

size_t Reaper::running() {
    return running_.size();  // TODO
}

void Reaper::wait_all() {
    // TODO: block SIGCHLD; collect and sigsuspend until running_ is empty.
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
