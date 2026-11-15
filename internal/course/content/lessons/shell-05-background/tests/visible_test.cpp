#include "doctest.h"

#include <chrono>
#include <regex>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("a background job: its number and PID on stderr, and the shell carries on at once") {
    Shell sh;
    auto start = std::chrono::steady_clock::now();
    Ran r = in_shell(sh, "sleep 0.3 &");
    CHECK_MESSAGE(std::chrono::steady_clock::now() - start < std::chrono::milliseconds(200),
                  "the shell waited for the background job");
    CHECK(r.status == 0);
    CHECK_MESSAGE(std::regex_match(r.err, std::regex(R"(\[1\] [0-9]+\n)")),
                  (std::string("expected [1] and a PID on stderr, got: ") + r.err));
    in_shell(sh, "wait");
}

TEST_CASE("a finished job is reported once, before the next line") {
    Ran r = conch("true &\nsleep 0.3\necho next\necho again\n");
    CHECK(r.out == "[1] Done  true\nnext\nagain\n");
}

}  // namespace shtest
