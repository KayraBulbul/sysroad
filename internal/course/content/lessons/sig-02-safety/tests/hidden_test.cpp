#include "doctest.h"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <string>

#include "helpers.hpp"
#include "safe.hpp"

static std::string line(const char* text, long n, size_t size = 64) {
    char buf[64];
    size_t len = format_line(buf, size, text, n);
    return std::string(buf, len);
}

TEST_CASE("format_line: the extremes of a long") {
    CHECK(line("", LONG_MAX) == std::to_string(LONG_MAX) + "\n");
    CHECK(line("", LONG_MIN) == std::to_string(LONG_MIN) + "\n");
    CHECK(line("x", 1000000) == "x1000000\n");
}

TEST_CASE("format_line: never writes past size") {
    char buf[16];
    for (char& c : buf) {
        c = '#';
    }
    size_t n = format_line(buf, 5, "abc", -123);
    CHECK(n == 5);
    CHECK(std::string(buf, 5) == "abc-1");
    CHECK(buf[5] == '#');
    CHECK(format_line(buf, 0, "abc", 1) == 0);
    CHECK(buf[0] == 'a');
    CHECK(format_line(buf, 9, "abc", -123) == 8);
}

TEST_CASE("an empty pipe: next() returns 0 at once") {
    int r = within(2, [] {
        SignalPipe sp({SIGUSR1});
        return sp.next() == 0 ? 0 : 1;
    });
    CHECK_MESSAGE(r != -1, "next() blocked: the read end must be non-blocking");
    CHECK(r == 0);
}

TEST_CASE("both ends are close-on-exec, and the handler restarts calls") {
    SignalPipe sp({SIGUSR1});
    CHECK((fcntl(sp.fd(), F_GETFD) & FD_CLOEXEC) != 0);
    CHECK((fcntl(sp.fd(), F_GETFL) & O_NONBLOCK) != 0);
    struct sigaction now{};
    sigaction(SIGUSR1, nullptr, &now);
    CHECK(now.sa_handler != SIG_DFL);
    CHECK((now.sa_flags & SA_RESTART) != 0);
    // The write end: every fd above the read end that's open now.
    for (int fd = 3; fd < 64; ++fd) {
        int flags = fcntl(fd, F_GETFD);
        if (flags != -1 && fd != sp.fd() && (fcntl(fd, F_GETFL) & O_NONBLOCK)) {
            CHECK((flags & FD_CLOEXEC) != 0);
        }
    }
}

TEST_CASE("the destructor puts things back") {
    int before = lowest_free_fd();
    struct sigaction term_before{};
    sigaction(SIGTERM, nullptr, &term_before);  // the test framework has one
    {
        SignalPipe sp({SIGUSR1, SIGTERM});
    }
    struct sigaction now{};
    sigaction(SIGUSR1, nullptr, &now);
    CHECK(now.sa_handler == SIG_DFL);
    sigaction(SIGTERM, nullptr, &now);
    CHECK(now.sa_handler == term_before.sa_handler);
    CHECK(lowest_free_fd() == before);
}

TEST_CASE("a full pipe doesn't block the handler, or change errno") {
    int r = within(5, [] {
        SignalPipe sp({SIGUSR1});
        for (int i = 0; i < 300'000; ++i) {
            raise(SIGUSR1);  // far more than the pipe holds
        }
        errno = EDOM;
        raise(SIGUSR1);
        if (errno != EDOM) {
            return 2;
        }
        int count = 0;
        while (sp.next() == SIGUSR1) {
            ++count;
        }
        return count >= 512 ? 0 : 3;
    });
    CHECK_MESSAGE(r != -1, "a raise hung: the write end must be non-blocking");
    CHECK_MESSAGE(r != 2, "the handler changed errno");
    CHECK(r == 0);
}

TEST_CASE("the handler keeps errno even when the write succeeds") {
    SignalPipe sp({SIGUSR2});
    errno = ERANGE;
    raise(SIGUSR2);
    CHECK(errno == ERANGE);
    CHECK(sp.next() == SIGUSR2);
}
