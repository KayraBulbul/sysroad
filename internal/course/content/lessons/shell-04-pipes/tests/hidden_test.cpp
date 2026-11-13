#include "doctest.h"

#include <fcntl.h>
#include <unistd.h>

#include <chrono>

#include "shell.hpp"

static int lowest_free_fd() {
    int fd = open("/dev/null", O_RDONLY);
    close(fd);
    return fd;
}

TEST_CASE("parse: redirections stay with their command") {
    List list = parse(tokenize("cat < in | sort > out 2>&1"));
    REQUIRE(list.size() == 1);
    const auto& cmds = list[0].pipeline.commands;
    REQUIRE(cmds.size() == 2);
    REQUIRE(cmds[0].redirects.size() == 1);
    CHECK(cmds[0].redirects[0].path == "in");
    CHECK(cmds[1].redirects.size() == 2);
}

TEST_CASE("parse: a trailing ; is fine, and an empty line is nothing") {
    List list = parse(tokenize("echo a;"));
    REQUIRE(list.size() == 1);
    CHECK(list[0].next == Step::End);
    CHECK(parse(tokenize("")).empty());
    CHECK(parse(tokenize("   # just a comment")).empty());
}

TEST_CASE("parse: syntax errors") {
    for (const char* line : {"| cat", "cat |", "a | | b", "; a", "a ;; b", "a &&", "|| b", "a && && b"}) {
        CHECK_THROWS_AS_MESSAGE(parse(tokenize(line)), SyntaxError, line);
    }
}

TEST_CASE("run_list: skipped commands keep $? as it was") {
    Shell sh;
    run_list(sh, parse(tokenize("false && echo no && echo no")));
    CHECK(sh.last_status == 1);
    run_list(sh, parse(tokenize("false || true || echo no")));
    CHECK(sh.last_status == 0);
    run_list(sh, parse(tokenize("true && false || true")));
    CHECK(sh.last_status == 0);
}

TEST_CASE("run_pipeline runs the commands at the same time") {
    Shell sh;
    auto start = std::chrono::steady_clock::now();
    run_list(sh, parse(tokenize("sleep 0.4 | sleep 0.4 | sleep 0.4")));
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::milliseconds(1000));
}

TEST_CASE("no pipe ends leak") {
    Shell sh;
    int before = lowest_free_fd();
    for (int i = 0; i < 20; ++i) {
        run_list(sh, parse(tokenize("true | true | true")));
    }
    CHECK(lowest_free_fd() == before);
}
