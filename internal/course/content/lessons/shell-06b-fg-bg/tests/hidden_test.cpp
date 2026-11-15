#include "doctest.h"

#include <unistd.h>

#include <fstream>

#include "conch_pty.hpp"

namespace shtest {

TEST_CASE("bg carries a stopped job on in the background") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 0.6");
    usleep(150'000);
    c.send("\x1a");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sleep 0.6"), c.output());
    REQUIRE(c.prompt());
    c.line("bg");
    CHECK_MESSAGE(c.expect("[1] sleep 0.6 &"), c.output());
    REQUIRE(c.prompt());
    usleep(800'000);
    c.line("");
    CHECK_MESSAGE(c.expect("[1] Done  sleep 0.6"), c.output());
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("fg %n picks a job; it may stop again and keeps its number") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5 | sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sleep 5 | sleep 5"), c.output());
    REQUIRE(c.prompt());
    c.line("sleep 6");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE(c.expect("[2] Stopped  sleep 6"));
    REQUIRE(c.prompt());
    c.line("fg %1");
    REQUIRE(c.expect("sleep 5 | sleep 5"));
    usleep(200'000);
    c.send("\x1a");
    CHECK_MESSAGE(c.expect("[1] Stopped  sleep 5 | sleep 5"), c.output());
    REQUIRE(c.prompt());
    c.line("fg %1");
    REQUIRE(c.expect("sleep 5 | sleep 5"));
    usleep(200'000);
    c.send("\x03");
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("jobs");
    CHECK_MESSAGE(c.expect("[2] Stopped  sleep 6"), c.output());
}

TEST_CASE("a background job that reads the terminal is stopped, and fg gives it the terminal") {
    {
        std::ofstream("reader.sh") << "read x\necho got $x\n";
    }
    Conch c;
    REQUIRE(c.prompt());
    c.line("sh reader.sh &");
    REQUIRE(c.prompt());
    usleep(300'000);
    c.line("jobs");
    CHECK_MESSAGE(c.expect("[1] Stopped  sh reader.sh"), c.output());
    REQUIRE(c.prompt());
    c.line("fg");
    REQUIRE(c.expect("sh reader.sh"));
    usleep(200'000);
    c.line("typed");
    CHECK_MESSAGE(c.expect("got typed"), c.output());
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("fg gives a stopped job back its own terminal settings") {
    {
        std::ofstream("modes.sh") << "stty -echo\n"
                                     "kill -TSTP $$\n"
                                     "if stty -a | grep -qw -- -echo; then echo kept-modes; "
                                     "else echo lost-modes; fi\n"
                                     "stty echo\n";
    }
    Conch c;
    REQUIRE(c.prompt());
    c.line("sh modes.sh");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sh modes.sh"), c.output());
    REQUIRE(c.prompt());
    c.line("fg");
    CHECK_MESSAGE(c.expect("kept-modes", 3000), c.output());
    REQUIRE(c.prompt());
    c.line("exit 0");
    CHECK(c.wait() == 0);
}

TEST_CASE("fg and bg with no such job") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("fg");
    CHECK_MESSAGE(c.expect("conch: fg:"), c.output());
    REQUIRE(c.prompt());
    c.line("bg %3");
    CHECK_MESSAGE(c.expect("conch: bg:"), c.output());
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK(c.wait() == 1);
}

}  // namespace shtest
