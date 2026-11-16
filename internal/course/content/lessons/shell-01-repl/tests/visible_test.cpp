#include "doctest.h"

#include "conch_pty.hpp"
#include "conch_run.hpp"

namespace shtest {

TEST_CASE("run_line runs a command and returns its status") {
    Shell sh;
    Ran hi = in_shell(sh, "echo hello there");
    CHECK(hi.out == "hello there\n");
    CHECK(hi.status == 0);
    CHECK(in_shell(sh, "false").status == 1);
    CHECK(in_shell(sh, "true").status == 0);
}

TEST_CASE("a command that isn't found: a message on stderr, status 127, and the shell carries on") {
    Ran r = conch("no-such-command-anywhere\necho after\n");
    CHECK(r.out == "after\n");
    CHECK(r.err == "conch: no-such-command-anywhere: command not found\n");
    Shell sh;
    CHECK(in_shell(sh, "no-such-command-anywhere").status == 127);
}

TEST_CASE("on a terminal, conch shows its prompt and runs what you type") {
    Conch c;
    REQUIRE_MESSAGE(c.prompt(), (std::string("expected the prompt \"conch$ \". The terminal showed: ") + c.output()));
    c.line("echo hello");
    CHECK_MESSAGE(c.expect("hello"), c.output());
    CHECK_MESSAGE(c.prompt(), (std::string("expected the prompt again after the command: ") + c.output()));
    c.send("\x04");  // Ctrl-D: the end of input
    int status = c.wait();
    CHECK_MESSAGE(status == 0, (std::string("conch should exit with $? (0) at the end of input. It showed: ") + c.output()));
}

}  // namespace shtest
