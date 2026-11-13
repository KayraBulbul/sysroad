#include "doctest.h"

#include <fcntl.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>

#include "shell.hpp"

using Words = std::vector<std::string>;

TEST_CASE("split_words: # only starts a comment at the start of a word") {
    CHECK(split_words("echo a#b #c d") == Words{"echo", "a#b"});
    CHECK(split_words("# all comment").empty());
    CHECK(split_words("   ").empty());
    CHECK(split_words("\tx\t") == Words{"x"});
}

TEST_CASE("run_command: not found is 127") {
    int saved = dup(2);
    int null = open("/dev/null", O_WRONLY);
    dup2(null, 2);
    close(null);
    int status = run_command({"surely-not-a-command-anywhere"});
    std::fflush(stderr);
    dup2(saved, 2);
    close(saved);
    CHECK(status == 127);
}

TEST_CASE("run_command: a signal is 128 + its number") {
    CHECK(run_command({"sh", "-c", "kill -TERM $$"}) == 128 + SIGTERM);
}

TEST_CASE("run_line sets $?") {
    Shell sh;
    run_line(sh, "false");
    CHECK(sh.last_status == 1);
    run_line(sh, "true");
    CHECK(sh.last_status == 0);
    run_line(sh, "   ");  // nothing to run: $? stays
    run_line(sh, "false");
    run_line(sh, "# just a comment");
    CHECK(sh.last_status == 1);
}
