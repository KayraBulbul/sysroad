#include "doctest.h"

#include <unistd.h>

#include "conch_pty.hpp"
#include "conch_run.hpp"

namespace shtest {

TEST_CASE("fg brings a stopped job back, and Ctrl-C ends it") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sleep 5"), c.output());
    REQUIRE(c.prompt());
    c.line("fg");
    CHECK_MESSAGE(c.expect("sleep 5"), c.output());
    usleep(200'000);
    c.send("\x03");
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("jobs");
    c.line("exit");
    CHECK_MESSAGE(c.wait() == 0, (std::string("the job was gone, so exit should leave at once: ") + c.output()));
}

TEST_CASE("exit warns once about stopped jobs, and leaves the second time") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE(c.expect("Stopped"));
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK_MESSAGE(c.expect("there are stopped jobs"), c.output());
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK(c.wait() == 1);  // $? from the first exit, which refused
}

TEST_CASE("without a terminal, fg and bg say there's no job control") {
    Ran r = conch("fg\nbg\n");
    CHECK(r.err == "conch: fg: no job control\nconch: bg: no job control\n");
    CHECK(r.status == 1);
}

}  // namespace shtest
