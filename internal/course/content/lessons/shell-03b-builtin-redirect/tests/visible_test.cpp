#include "doctest.h"

#include <fcntl.h>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("a redirected builtin writes to the file, and the shell's own stdout comes back") {
    Shell sh;
    int before = lowest_free_fd();
    Ran r = in_shell(sh, "type cd > saved.txt");
    CHECK(r.out == "");
    CHECK(read_file("saved.txt") == "cd is a shell builtin\n");
    CHECK_MESSAGE(in_shell(sh, "type pwd").out == "pwd is a shell builtin\n", "the shell's stdout wasn't put back");
    CHECK_MESSAGE(lowest_free_fd() == before, "the shell has an fd open that it didn't have before");
}

TEST_CASE("a redirection that fails leaves the shell's fds as they were") {
    Shell sh;
    int before = lowest_free_fd();
    Ran r = in_shell(sh, "type cd > ok.txt > no/such/dir/x");
    CHECK(r.status == 1);
    CHECK(r.err == "conch: no/such/dir/x: No such file or directory\n");
    CHECK(in_shell(sh, "type pwd").out == "pwd is a shell builtin\n");
    CHECK(lowest_free_fd() == before);
}

}  // namespace shtest
