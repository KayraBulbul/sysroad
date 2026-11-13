#include "doctest.h"

#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <sstream>
#include <string>
#include <system_error>

#include "signals.hpp"

static void nothing(int) {}
static void other(int) {}

TEST_CASE("SIGKILL and SIGSTOP can't be caught") {
    try {
        install_handler(SIGKILL, nothing, 0);
        FAIL("no exception");
    } catch (const std::system_error& e) {
        CHECK(e.code().value() == EINVAL);
    }
    CHECK_THROWS_AS(install_handler(SIGSTOP, nothing, 0), std::system_error);
}

TEST_CASE("install_handler returns the action it replaced") {
    struct sigaction first = install_handler(SIGUSR2, nothing, 0);
    struct sigaction second = install_handler(SIGUSR2, other, SA_RESTART);
    CHECK(second.sa_handler == nothing);
    struct sigaction now{};
    sigaction(SIGUSR2, nullptr, &now);
    CHECK(now.sa_handler == other);
    CHECK((now.sa_flags & SA_RESTART) != 0);
    CHECK(sigismember(&now.sa_mask, SIGINT) == 0);
    CHECK(sigismember(&now.sa_mask, SIGTERM) == 0);
    sigaction(SIGUSR2, &first, nullptr);
}

TEST_CASE("SIG_IGN works as a handler too") {
    struct sigaction old = install_handler(SIGUSR2, SIG_IGN, 0);
    raise(SIGUSR2);
    CHECK(true);  // still alive
    sigaction(SIGUSR2, &old, nullptr);
}

// Reads one byte from a pipe that a child fills after 300 ms, while a
// SIGALRM arrives after 100 ms.
static ssize_t read_through_an_alarm(int flags, int& err) {
    struct sigaction old = install_handler(SIGALRM, nothing, flags);
    int p[2];
    pipe(p);
    pid_t child = fork();
    if (child == 0) {
        usleep(300'000);
        write(p[1], "x", 1);
        _exit(0);
    }
    close(p[1]);
    itimerval t{};
    t.it_value.tv_usec = 100'000;
    setitimer(ITIMER_REAL, &t, nullptr);
    char c;
    errno = 0;
    ssize_t n = read(p[0], &c, 1);
    err = errno;
    close(p[0]);
    waitpid(child, nullptr, 0);
    sigaction(SIGALRM, &old, nullptr);
    return n;
}

TEST_CASE("the flags are passed on: SA_RESTART, or EINTR without it") {
    int err = 0;
    CHECK(read_through_an_alarm(SA_RESTART, err) == 1);
    CHECK(read_through_an_alarm(0, err) == -1);
    CHECK(err == EINTR);
}

TEST_CASE("tick_until_stopped: already stopped") {
    stop_signal = SIGINT;
    std::ostringstream out;
    CHECK(tick_until_stopped(10, out) == 0);
    CHECK(out.str().empty());
    stop_signal = 0;
}

TEST_CASE("tick_until_stopped: the sleep that a signal cuts short isn't a tick") {
    stop_signal = 0;
    struct sigaction old = install_handler(SIGALRM, stop_handler, 0);
    itimerval t{};
    t.it_value.tv_usec = 150'000;
    setitimer(ITIMER_REAL, &t, nullptr);
    std::ostringstream out;
    int ticks = tick_until_stopped(1000, out);
    sigaction(SIGALRM, &old, nullptr);
    CHECK(ticks == 0);
    CHECK(out.str().empty());
    stop_signal = 0;
}

TEST_CASE("tick_until_stopped counts what it printed") {
    stop_signal = 0;
    struct sigaction old = install_handler(SIGALRM, stop_handler, 0);
    itimerval t{};
    t.it_value.tv_usec = 230'000;
    setitimer(ITIMER_REAL, &t, nullptr);
    std::ostringstream out;
    int ticks = tick_until_stopped(20, out);
    sigaction(SIGALRM, &old, nullptr);
    std::string expect;
    for (int i = 1; i <= ticks; ++i) {
        expect += "tick " + std::to_string(i) + "\n";
    }
    CHECK(ticks >= 5);
    CHECK(out.str() == expect);
    stop_signal = 0;
}
