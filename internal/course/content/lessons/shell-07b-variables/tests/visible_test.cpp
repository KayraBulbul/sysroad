#include "doctest.h"

#include <cstdlib>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("NAME=value alone sets a shell variable, which commands don't see until it's exported") {
    unsetenv("SR_V");
    Shell sh;
    CHECK(in_shell(sh, "SR_V=one").status == 0);
    CHECK(in_shell(sh, "echo $SR_V").out == "one\n");
    CHECK(std::getenv("SR_V") == nullptr);
    CHECK(in_shell(sh, "printenv SR_V").status == 1);
    in_shell(sh, "export SR_V");
    CHECK(in_shell(sh, "printenv SR_V").out == "one\n");
    in_shell(sh, "unset SR_V");
}

TEST_CASE("NAME=value before a command is in its environment, and only its") {
    unsetenv("SR_T");
    Shell sh;
    CHECK(in_shell(sh, "SR_T=temp printenv SR_T").out == "temp\n");
    CHECK(in_shell(sh, "echo [$SR_T]").out == "[]\n");
    CHECK(std::getenv("SR_T") == nullptr);
}

}  // namespace shtest
