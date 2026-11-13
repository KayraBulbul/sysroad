#include "doctest.h"

#include <sys/wait.h>
#include <unistd.h>

#include "helpers.hpp"
#include "mask.hpp"

static volatile sig_atomic_t hits = 0;
static void hit(int) {
    hits = hits + 1;
}

TEST_CASE("a blocked signal waits until it's unblocked") {
    hits = 0;
    struct sigaction sa{}, old{};
    sa.sa_handler = hit;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, &old);
    {
        BlockSignals block({SIGUSR1});
        raise(SIGUSR1);
        CHECK(hits == 0);
        CHECK(is_pending(SIGUSR1));
    }
    CHECK(hits == 1);
    CHECK_FALSE(is_pending(SIGUSR1));
    sigaction(SIGUSR1, &old, nullptr);
}

TEST_CASE("await a signal from a child") {
    int r = within(3, [] {
        BlockSignals block({SIGUSR2});
        pid_t child = fork();
        if (child == 0) {
            usleep(200'000);
            kill(getppid(), SIGUSR2);
            _exit(0);
        }
        int sig = await_signal({SIGUSR2});
        waitpid(child, nullptr, 0);
        return sig == SIGUSR2 ? 0 : 1;
    });
    CHECK_MESSAGE(r != -1, "await_signal never returned");
    CHECK(r == 0);
}
