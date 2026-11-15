#include "doctest.h"

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("a file that can't be opened: a message naming it, and status 1") {
    Ran r = conch("cat < missing.txt\n");
    CHECK(r.err == "conch: missing.txt: No such file or directory\n");
    CHECK(r.status == 1);
}

TEST_CASE("a syntax error: a message, status 2, and the shell carries on") {
    Ran r = conch("echo >\n");
    CHECK(r.status == 2);
    CHECK(r.err.rfind("conch: syntax error:", 0) == 0);
    CHECK(conch("echo >\necho after\n").out == "after\n");
}

}  // namespace shtest
