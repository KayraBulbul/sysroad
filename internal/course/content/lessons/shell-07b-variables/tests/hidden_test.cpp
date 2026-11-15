#include "doctest.h"

#include <cstdlib>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("a shell variable comes before an environment variable of the same name") {
    setenv("SR_BOTH", "env", 1);
    unsetenv("SR_ONLY");
    Shell sh;
    in_shell(sh, "SR_ONLY=local");
    CHECK(in_shell(sh, "echo $SR_ONLY $SR_BOTH").out == "local env\n");
    unsetenv("SR_BOTH");
}

TEST_CASE("only an unquoted name before = makes an assignment") {
    Shell sh;
    for (const char* line : {"1A=x", "=x", "A-B=x"}) {
        INFO(line);
        CHECK(in_shell(sh, line).status == 127);  // run as a command, and not found
    }
    CHECK(in_shell(sh, "A_1=x").status == 0);
    CHECK(in_shell(sh, "_=x").status == 0);
    CHECK(in_shell(sh, "SR_Q='x y'").status == 0);
    CHECK(in_shell(sh, "printf [%s] $SR_Q").out == "[x y]");
}

TEST_CASE("after a command word, NAME=value is an ordinary word") {
    Shell sh;
    CHECK(in_shell(sh, "echo A=1").out == "A=1\n");
}

TEST_CASE("export of a name with no value, set or not") {
    unsetenv("SR_NEW");
    Shell sh;
    in_shell(sh, "export SR_NEW");
    REQUIRE(std::getenv("SR_NEW") != nullptr);
    CHECK(std::string(std::getenv("SR_NEW")).empty());
    unsetenv("SR_NEW");
}

}  // namespace shtest
