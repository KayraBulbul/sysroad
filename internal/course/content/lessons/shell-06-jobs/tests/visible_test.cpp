#include "doctest.h"

#include "conch_pty.hpp"

namespace shtest {

TEST_CASE("Ctrl-C ends the job, and not the shell; $? is 130") {
    Conch c;
    REQUIRE(c.prompt());
    std::string why = c.start_job("sleep 5");
    REQUIRE_MESSAGE(why.empty(), why);
    c.send("\x03");
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("echo still alive");
    CHECK_MESSAGE(c.expect("still alive"), c.output());
    REQUIRE(c.prompt());
    why = c.start_job("sleep 5");
    REQUIRE_MESSAGE(why.empty(), why);
    c.send("\x03");
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK_MESSAGE(c.wait() == 130, (std::string("exit with no number uses $?, 130 after Ctrl-C: ") + c.output()));
}

TEST_CASE("Ctrl-Z stops the job: it goes in the table, and jobs lists it as Stopped") {
    Conch c;
    REQUIRE(c.prompt());
    std::string why = c.start_job("sleep 5");
    REQUIRE_MESSAGE(why.empty(), why);
    c.send("\x1a");
    CHECK_MESSAGE(c.expect("[1] Stopped  sleep 5"), c.output());
    REQUIRE(c.prompt());
    CHECK_MESSAGE(c.shell_in_foreground(), "after the job stopped, the shell should take the terminal back");
    c.line("jobs");
    CHECK_MESSAGE(c.expect("[1] Stopped  sleep 5"), c.output());
    REQUIRE(c.prompt());
}

}  // namespace shtest
