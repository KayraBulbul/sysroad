#include "doctest.h"

#include <poll.h>

#include <string>

#include "safe.hpp"

static std::string line(const char* text, long n, size_t size = 64) {
    char buf[64];
    size_t len = format_line(buf, size, text, n);
    return std::string(buf, len);
}

TEST_CASE("format_line") {
    CHECK(line("count: ", 42) == "count: 42\n");
    CHECK(line("", 0) == "0\n");
    CHECK(line("delta ", -17) == "delta -17\n");
}

TEST_CASE("signals come out of the pipe in order") {
    SignalPipe sp({SIGUSR1, SIGUSR2});
    raise(SIGUSR1);
    raise(SIGUSR2);
    pollfd p{sp.fd(), POLLIN, 0};
    CHECK(poll(&p, 1, 0) == 1);
    CHECK(sp.next() == SIGUSR1);
    CHECK(sp.next() == SIGUSR2);
    CHECK(sp.next() == 0);
}
