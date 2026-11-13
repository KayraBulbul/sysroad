#include "mask.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <vector>

BlockSignals::BlockSignals(std::initializer_list<int> sigs) {
    // TODO: build a sigset_t from sigs, and SIG_BLOCK it, keeping the old mask.
    (void)sigs;
    sigemptyset(&old_);
}

BlockSignals::~BlockSignals() {
    // TODO
}

bool is_pending(int sig) {
    (void)sig;  // TODO: sigpending
    return false;
}

int await_signal(std::initializer_list<int> sigs) {
    // TODO: block sigs, install a handler that notes the signal, then
    // sigsuspend with a mask that lets them in, until one is noted.
    (void)sigs;
    return 0;
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
