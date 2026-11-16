#include "doctest.h"

#include <cstdlib>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("inside double quotes, a backslash only escapes \" \\ $ and `") {
    setenv("X", "v", 1);
    Shell sh;
    CHECK(in_shell(sh, "printf [%s] \"\\$X \\\" \\\\ \\n \\`\"").out == "[$X \" \\ \\n `]");
    CHECK(in_shell(sh, "printf [%s] \"'$X'\" '\"$X\"'").out == "['v'][\"$X\"]");
}

TEST_CASE("$! is the last background job's PID") {
    Shell sh;
    Ran started = in_shell(sh, "sleep 0.1 &");
    std::string pid = started.err.substr(started.err.find(' ') + 1);
    CHECK(in_shell(sh, "echo $!").out == pid);
    in_shell(sh, "wait");
}

TEST_CASE("a name runs as far as letters, digits and _ go") {
    setenv("N", "n", 1);
    unsetenv("N_");
    Shell sh;
    CHECK(in_shell(sh, "printf [%s] $N.$N $N_ x").out == "[n.n][x]");
}

TEST_CASE("${ with no } is a syntax error, when the command runs") {
    Shell sh;
    Ran r = in_shell(sh, "echo ${N");
    CHECK(r.status == 2);
    CHECK(r.err.rfind("conch: syntax error:", 0) == 0);
}

TEST_CASE("~ only at the very start of an unquoted word") {
    setenv("HOME", "/h", 1);
    Shell sh;
    CHECK(in_shell(sh, "printf [%s] ~ ~/a x~ '~' \"~\" ~user").out == "[/h][/h/a][x~][~][~][~user]");
}

TEST_CASE("a word made only of unset variables vanishes; quotes keep it") {
    unsetenv("SR_NOT_SET");
    Shell sh;
    CHECK(in_shell(sh, "printf [%s] $SR_NOT_SET$SR_NOT_SET x").out == "[x]");
    CHECK(in_shell(sh, "printf [%s] ''$SR_NOT_SET x").out == "[][x]");
    CHECK(in_shell(sh, "printf [%s] \\$SR_NOT_SET").out == "[$SR_NOT_SET]");
}

TEST_CASE("a command made only of unset variables runs nothing") {
    unsetenv("SR_NOT_SET");
    Shell sh;
    in_shell(sh, "false");
    Ran r = in_shell(sh, "$SR_NOT_SET");
    CHECK(r.err == "");
    CHECK(r.status == 0);
}

}  // namespace shtest
