#include "doctest.h"

#include <unistd.h>

#include <climits>
#include <cstdlib>
#include <string>

#include "shell.hpp"

TEST_CASE("the builtins exist, and nothing else does") {
    for (const char* name : {"cd", "pwd", "exit", "export", "unset", "type"}) {
        CHECK_MESSAGE(find_builtin(name) != nullptr, name);
    }
    CHECK(find_builtin("ls") == nullptr);
    CHECK(find_builtin("") == nullptr);
}

TEST_CASE("cd changes the shell's own folder") {
    char before[PATH_MAX];
    REQUIRE(getcwd(before, sizeof before) != nullptr);
    Shell sh;
    CHECK(find_builtin("cd")(sh, {"cd", "/"}) == 0);
    char now[PATH_MAX];
    REQUIRE(getcwd(now, sizeof now) != nullptr);
    CHECK(std::string(now) == "/");
    chdir(before);
}

TEST_CASE("exit sets the shell to leave") {
    Shell sh;
    find_builtin("exit")(sh, {"exit", "7"});
    CHECK(sh.exiting);
    CHECK(sh.exit_code == 7);
}
