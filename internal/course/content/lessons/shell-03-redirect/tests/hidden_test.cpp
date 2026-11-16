#include "doctest.h"

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("copying an fd that isn't open names the fd") {
    Ran r = conch("echo x >&57\n");
    CHECK(r.err == "conch: 57: Bad file descriptor\n");
    CHECK(r.status == 1);
    CHECK(r.out == "");
}

TEST_CASE("syntax errors: a target that's missing, an operator, or not a number") {
    for (const char* line : {"echo >", "echo > < x", "echo >& file", "cat <&", "cat 2>"}) {
        INFO(line);
        Ran r = conch(std::string(line) + "\n");
        CHECK(r.status == 2);
        CHECK(r.err.rfind("conch: syntax error:", 0) == 0);
    }
}

TEST_CASE("a file conch can't create") {
    Ran r = conch("echo x > no/such/folder/f.txt\necho after\n");
    CHECK(r.err == "conch: no/such/folder/f.txt: No such file or directory\n");
    CHECK(r.out == "after\n");
}

TEST_CASE("> truncates and >> appends, and both create with the usual mode") {
    write_file("t.txt", "a long old line\n");
    Ran r = conch("echo new > t.txt\necho more >> t.txt\ncat t.txt\n");
    CHECK(r.out == "new\nmore\n");
}

}  // namespace shtest
