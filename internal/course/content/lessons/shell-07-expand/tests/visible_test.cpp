#include "doctest.h"

#include <unistd.h>

#include <cstdlib>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("quotes and backslashes keep a word together, and are removed") {
    Shell sh;
    CHECK(in_shell(sh, "printf [%s] 'a b' \"c | d\" e\\ f").out == "[a b][c | d][e f]");
    CHECK(in_shell(sh, "printf [%s] \"a\"'b'c").out == "[abc]");
}

TEST_CASE("$NAME, ${NAME}, $? and $$") {
    setenv("SR_SEVEN", "seven", 1);
    Shell sh;
    CHECK(in_shell(sh, "printf [%s] $SR_SEVEN ${SR_SEVEN}s").out == "[seven][sevens]");
    in_shell(sh, "false");
    CHECK(in_shell(sh, "echo $?").out == "1\n");
    CHECK(in_shell(sh, "echo $$").out == std::to_string(getpid()) + "\n");
}

TEST_CASE("an unclosed quote is a syntax error") {
    Ran r = conch("echo 'abc\n");
    CHECK(r.status == 2);
    CHECK(r.err.rfind("conch: syntax error:", 0) == 0);
}

}  // namespace shtest
