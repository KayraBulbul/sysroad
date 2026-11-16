#include "doctest.h"

#include <csignal>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("a blank line, or a comment, leaves $? as it was") {
    Shell sh;
    in_shell(sh, "false");
    CHECK(in_shell(sh, "").status == 1);
    CHECK(in_shell(sh, "   \t ").status == 1);
    CHECK(in_shell(sh, "# just a comment").status == 1);
    CHECK(in_shell(sh, "true # and a comment").status == 0);
}

TEST_CASE("a command killed by a signal: 128 plus the signal's number") {
    write_file("term.sh", "kill -TERM $$\n");
    Shell sh;
    CHECK(in_shell(sh, "sh term.sh").status == 128 + SIGTERM);
}

TEST_CASE("a file that can't be run: a message, and status 126") {
    write_file("plain.txt", "not a program\n");  // not executable
    Shell sh;
    Ran r = in_shell(sh, "./plain.txt");
    CHECK(r.status == 126);
    CHECK(r.err == "conch: ./plain.txt: Permission denied\n");
}

TEST_CASE("words are split at any run of spaces and tabs") {
    Shell sh;
    CHECK(in_shell(sh, "  printf [%s]  a \t b\t\tc  ").out == "[a][b][c]");
    CHECK(in_shell(sh, "printf [%s] a#b #c d").out == "[a#b]");
}

}  // namespace shtest
