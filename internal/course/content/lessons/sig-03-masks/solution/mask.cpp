#include "mask.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <vector>

static sigset_t set_of(std::initializer_list<int> sigs) {
    sigset_t set;
    sigemptyset(&set);
    for (int sig : sigs) {
        sigaddset(&set, sig);
    }
    return set;
}

BlockSignals::BlockSignals(std::initializer_list<int> sigs) {
    sigset_t set = set_of(sigs);
    sigprocmask(SIG_BLOCK, &set, &old_);
}

BlockSignals::~BlockSignals() {
    sigprocmask(SIG_SETMASK, &old_, nullptr);
}

bool is_pending(int sig) {
    sigset_t pending;
    sigpending(&pending);
    return sigismember(&pending, sig) == 1;
}

static volatile sig_atomic_t noted = 0;

static void note(int sig) {
    noted = sig;
}

int await_signal(std::initializer_list<int> sigs) {
    std::vector<std::pair<int, struct sigaction>> old;
    int got = 0;
    {
        BlockSignals block(sigs);
        for (int sig : sigs) {
            struct sigaction sa{};
            sa.sa_handler = note;
            sigemptyset(&sa.sa_mask);
            struct sigaction before{};
            sigaction(sig, &sa, &before);
            old.emplace_back(sig, before);
        }
        // While we sleep, let in exactly the signals we're waiting for.
        sigset_t during = block.previous();
        for (int sig : sigs) {
            sigdelset(&during, sig);
        }
        noted = 0;
        while (noted == 0) {
            sigsuspend(&during);  // unblocks and sleeps in one step: no gap
        }
        got = noted;
    }
    // The mask is back, so the actions can go back too.
    for (const auto& [sig, before] : old) {
        sigaction(sig, &before, nullptr);
    }
    return got;
}

// waitsig: forks 1000 children that each signal this process straight
// away, and waits for every signal. Blocking SIGUSR1 before the fork is
// what makes that safe.
int main() {
    auto start = std::chrono::steady_clock::now();
    int caught = 0;
    for (int i = 0; i < 1000; ++i) {
        BlockSignals block({SIGUSR1});
        pid_t child = fork();
        if (child == 0) {
            kill(getppid(), SIGUSR1);
            _exit(0);
        }
        if (await_signal({SIGUSR1}) == SIGUSR1) {
            ++caught;
        }
        waitpid(child, nullptr, 0);
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - start)
                  .count();
    std::cout << "1000 children each sent SIGUSR1 straight after the fork; caught " << caught
              << " in " << ms << " ms\n";
    return caught == 1000 ? 0 : 1;
}
