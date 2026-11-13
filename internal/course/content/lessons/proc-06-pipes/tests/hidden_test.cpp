#include "doctest.h"

#include <fcntl.h>
#include <unistd.h>

#include <csignal>
#include <string>

#include "capture.hpp"

static int lowest_free_fd() {
    int fd = ::open("/dev/null", O_RDONLY);
    ::close(fd);
    return fd;
}

static std::string big(size_t n) {
    std::string s;
    s.reserve(n);
    for (size_t i = 0; s.size() < n; ++i) {
        s += "line " + std::to_string(i) + "\n";
    }
    s.resize(n);
    return s;
}

TEST_CASE("capture: more output than a pipe holds") {
    Output o = capture({"head", "-c", "1000000", "/dev/zero"});
    CHECK(o.out.size() == 1000000);
    CHECK(o.status.ok());
}

TEST_CASE("capture: nothing at all") {
    Output o = capture({"true"});
    CHECK(o.out.empty());
    CHECK(o.status.ok());
}

TEST_CASE("capture: binary output, zero bytes and all") {
    Output o = capture({"printf", "a\\000b\\377"});
    CHECK(o.out == std::string("a\0b\xff", 4));
}

TEST_CASE("capture: stderr isn't captured") {
    Output o = capture({"sh", "-c", "echo out; echo err >&2 2>/dev/null"});
    CHECK(o.out == "out\n");
}

TEST_CASE("capture: a command that isn't found") {
    Output o = capture({"surely-not-a-command"});
    CHECK(o.out.empty());
    CHECK(o.status.code == 127);
}

TEST_CASE("communicate: megabytes both ways") {
    std::string in = big(3'000'000);
    Output o = communicate({"cat"}, in);
    CHECK(o.status.ok());
    CHECK(o.out.size() == in.size());
    CHECK(o.out == in);
}

TEST_CASE("communicate: a command that prints before it reads") {
    std::string in = big(500'000);
    Output o = communicate({"sh", "-c", "head -c 300000 /dev/zero; cat >/dev/null"}, in);
    CHECK(o.status.ok());
    CHECK(o.out.size() == 300000);
}

TEST_CASE("communicate: a command that never reads its input") {
    std::string in = big(500'000);
    Output o = communicate({"sh", "-c", "head -c 300000 /dev/zero"}, in);
    CHECK(o.status.ok());
    CHECK(o.out.size() == 300000);
}

TEST_CASE("communicate: a command that stops reading early doesn't kill us") {
    std::string in = big(1'000'000);
    Output o = communicate({"head", "-c", "5"}, in);
    CHECK(o.out == "line ");
    CHECK(o.status.ok());
}

TEST_CASE("communicate: empty input closes stdin at once") {
    Output o = communicate({"cat"}, "");
    CHECK(o.out.empty());
    CHECK(o.status.ok());
}

TEST_CASE("communicate puts SIGPIPE back the way it was") {
    struct sigaction before {};
    ::sigaction(SIGPIPE, nullptr, &before);
    communicate({"head", "-c", "1"}, big(200'000));
    struct sigaction after {};
    ::sigaction(SIGPIPE, nullptr, &after);
    CHECK(after.sa_handler == before.sa_handler);
}

TEST_CASE("children start with SIGPIPE's default action") {
    auto old = ::signal(SIGPIPE, SIG_IGN);  // as Python programs run
    Output a = capture({"sh", "-c", "kill -PIPE $$; echo survived"});
    Output b = communicate({"sh", "-c", "kill -PIPE $$; echo survived"}, "");
    ::signal(SIGPIPE, old);
    CHECK(a.out == "");
    CHECK(a.status.kind == Status::Signaled);
    CHECK(a.status.code == SIGPIPE);
    CHECK(b.status.kind == Status::Signaled);
    CHECK(b.status.code == SIGPIPE);
}

TEST_CASE("no fds leak") {
    int before = lowest_free_fd();
    for (int i = 0; i < 100; ++i) {
        capture({"true"});
        communicate({"cat"}, "x");
    }
    CHECK(lowest_free_fd() == before);
}
