#include "doctest.h"

#include "shell.hpp"

using Words = std::vector<std::string>;

TEST_CASE("split_words") {
    CHECK(split_words("ls -l /tmp") == Words{"ls", "-l", "/tmp"});
    CHECK(split_words("  echo \t hi  ") == Words{"echo", "hi"});
    CHECK(split_words("").empty());
    CHECK(split_words("echo hi # a comment") == Words{"echo", "hi"});
}

TEST_CASE("run_command returns the status") {
    CHECK(run_command({"true"}) == 0);
    CHECK(run_command({"false"}) == 1);
}
