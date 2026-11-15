#include "doctest.h"

#include <unistd.h>

#include "conch_pty.hpp"

namespace shtest {

TEST_CASE("Ctrl-C ends the job, and not the shell; $? is 130") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x03");
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("echo still alive");
    CHECK_MESSAGE(c.expect("still alive"), c.output());
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x03");
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK_MESSAGE(c.wait() == 130, (std::string("exit with no number uses $?, 130 after Ctrl-C: ") + c.output()));
}

TEST_CASE("Ctrl-Z stops the job: it goes in the table, and jobs lists it as Stopped") {
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
}

}  // namespace shtest
