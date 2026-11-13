#include "doctest.h"

#include "conch_pty.hpp"

TEST_CASE("Ctrl-Z stops a job, jobs lists it, fg brings it back, Ctrl-C ends it") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x1a");
    CHECK_MESSAGE(c.expect("[1] Stopped  sleep 5"), c.output());
    REQUIRE(c.prompt());
    c.line("jobs");
    CHECK_MESSAGE(c.expect("[1] Stopped  sleep 5"), c.output());
    REQUIRE(c.prompt());
    c.line("fg");
    CHECK_MESSAGE(c.expect("sleep 5"), c.output());
    usleep(200'000);
    c.send("\x03");
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("exit");
    CHECK_MESSAGE(c.wait() == 130, c.output());  // $? from the Ctrl-C
}

TEST_CASE("Ctrl-C ends the job, and not the shell") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x03");
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("echo still alive");
    CHECK_MESSAGE(c.expect("still alive"), c.output());
    c.line("exit 0");
    CHECK(c.wait() == 0);
}
