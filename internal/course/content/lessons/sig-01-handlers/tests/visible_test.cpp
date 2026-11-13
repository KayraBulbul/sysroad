#include "doctest.h"

#include <sys/time.h>
#include <unistd.h>

#include <sstream>

#include "signals.hpp"

static volatile sig_atomic_t usr1_count = 0;
static void count_usr1(int) {
    usr1_count = usr1_count + 1;
}

TEST_CASE("install a handler, and raise the signal") {
    usr1_count = 0;
    struct sigaction old = install_handler(SIGUSR1, count_usr1, 0);
    CHECK(old.sa_handler == SIG_DFL);
    raise(SIGUSR1);
    raise(SIGUSR1);
    CHECK(usr1_count == 2);
    sigaction(SIGUSR1, &old, nullptr);
}

TEST_CASE("stop_handler: SIGTERM no longer kills us") {
    stop_signal = 0;
    struct sigaction old = install_handler(SIGTERM, stop_handler, 0);
    kill(getpid(), SIGTERM);
    CHECK(stop_signal == SIGTERM);
    sigaction(SIGTERM, &old, nullptr);
    stop_signal = 0;
}

TEST_CASE("tick until an alarm stops it") {
    stop_signal = 0;
    struct sigaction old = install_handler(SIGALRM, stop_handler, 0);
    itimerval t{};
    t.it_value.tv_usec = 350'000;
    setitimer(ITIMER_REAL, &t, nullptr);
    std::ostringstream out;
    int ticks = tick_until_stopped(100, out);
    sigaction(SIGALRM, &old, nullptr);
    CHECK(stop_signal == SIGALRM);
    CHECK(ticks >= 2);
    CHECK(ticks <= 4);
    CHECK(out.str().rfind("tick 1\ntick 2\n", 0) == 0);
    stop_signal = 0;
}
