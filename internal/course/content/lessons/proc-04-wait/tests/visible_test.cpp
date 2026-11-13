#include "doctest.h"

#include <csignal>

#include "status.hpp"

TEST_CASE("an exit code") {
    Status s = wait_for(start({"sh", "-c", "exit 3"}));
    CHECK(s.kind == Status::Exited);
    CHECK(s.code == 3);
    CHECK_FALSE(s.ok());
    CHECK(describe(s) == "exited 3");
    CHECK(shell_status(s) == 3);
}

TEST_CASE("killed by a signal") {
    Status s = wait_for(start({"sh", "-c", "kill -TERM $$"}));
    CHECK(s.kind == Status::Signaled);
    CHECK(s.code == SIGTERM);
    CHECK(describe(s) == "killed by SIGTERM");
    CHECK(shell_status(s) == 128 + SIGTERM);
}

TEST_CASE("success") {
    Status s = wait_for(start({"true"}));
    CHECK(s.ok());
    CHECK(describe(s) == "exited 0");
}
