#include "doctest.h"

#include <unistd.h>

#include <climits>
#include <string>

#include "conch_run.hpp"

namespace shtest {

// Where the tests run, to come back to after a cd.
inline std::string here() {
    char dir[PATH_MAX];
    return getcwd(dir, sizeof dir) != nullptr ? dir : "";
}

TEST_CASE("cd changes the shell's own folder") {
    std::string start = here();
    Shell sh;
    CHECK(in_shell(sh, "cd /").status == 0);
    CHECK_MESSAGE(here() == "/", "cd ran in a child, so the shell didn't move");
    CHECK(in_shell(sh, "pwd").out == "/\n");
    chdir(start.c_str());
}

TEST_CASE("a builtin's errors go to stderr, and its status is $?") {
    Ran r = conch("cd /no/such/folder\n");
    CHECK(r.out == "");
    CHECK(r.err == "conch: cd: /no/such/folder: No such file or directory\n");
    CHECK(r.status == 1);
}

TEST_CASE("exit stops conch, with its number") {
    Ran r = conch("echo before\nexit 3\necho never\n");
    CHECK(r.out == "before\n");
    CHECK(r.status == 3);
}

}  // namespace shtest
