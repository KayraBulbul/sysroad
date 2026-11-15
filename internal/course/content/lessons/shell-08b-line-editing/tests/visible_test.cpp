#include "doctest.h"

#include "editing.hpp"

namespace shtest {

TEST_CASE("Left moves into the line, typing goes in there, and Backspace deletes") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("echo [wrld]" + times(keys::left, 4) + "o");
    c.send(keys::enter);
    CHECK_MESSAGE(printed(c, "[world]"), c.output());
    REQUIRE(c.prompt());
    c.send("echo [abcc]" + keys::left + keys::backspace + keys::enter);
    CHECK_MESSAGE(printed(c, "[abc]"), c.output());
}

TEST_CASE("Up brings back earlier lines, and Down the line you were typing") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("echo [one]");
    REQUIRE(printed(c, "[one]"));
    REQUIRE(c.prompt());
    c.line("echo [two]");
    REQUIRE(printed(c, "[two]"));
    REQUIRE(c.prompt());
    c.send(keys::up + keys::up + keys::enter);
    CHECK_MESSAGE(printed(c, "[one]"), c.output());
    REQUIRE(c.prompt());
    c.send("echo [dra" + keys::up + keys::down + "ft]" + keys::enter);
    CHECK_MESSAGE(printed(c, "[draft]"), c.output());
}

TEST_CASE("Ctrl-D on an empty line ends conch") {
    Conch c;
    REQUIRE(c.prompt());
    c.send(keys::ctrl_d);
    CHECK(c.wait() == 0);
}

}  // namespace shtest
