#include "doctest.h"

#include <cerrno>
#include <string>
#include <vector>

#include "env.hpp"
#include "helpers.hpp"

using Args = std::vector<std::string>;

TEST_CASE("parse: options, assignments, then the command") {
    EnvCommand cmd = parse_env_args({"-i", "-u", "HOME", "A=1", "B=x=y", "ls", "-l", "C=3"});
    CHECK(cmd.clear);
    CHECK(cmd.unset == Args{"HOME"});
    REQUIRE(cmd.set.size() == 2);
    CHECK(cmd.set[0].first == "A");
    CHECK(cmd.set[0].second == "1");
    CHECK(cmd.set[1].first == "B");
    CHECK(cmd.set[1].second == "x=y");
    CHECK(cmd.argv == Args{"ls", "-l", "C=3"});
}

TEST_CASE("build_environment") {
    EnvCommand cmd;
    cmd.unset = {"HOME"};
    cmd.set = {{"A", "9"}, {"NEW", "1"}};
    Args current{"A=1", "HOME=/home/me", "B=2"};
    CHECK(build_environment(current, cmd) == Args{"A=9", "B=2", "NEW=1"});
    cmd.clear = true;
    CHECK(build_environment(current, cmd) == Args{"A=9", "NEW=1"});
}

TEST_CASE("find_in_path") {
    ::mkdir("bin1", 0755);
    ::mkdir("bin2", 0755);
    write_file("bin2/tool", "#!/bin/sh\n", 0755);
    CHECK(find_in_path("tool", "bin1:bin2") == "bin2/tool");
    CHECK(find_in_path("nope", "bin1:bin2") == "");
    CHECK(find_in_path("./anything", "bin1") == "./anything");
}

TEST_CASE("exec_with runs the program with that environment") {
    int code = in_child([] {
        exec_with({"sh", "-c", "echo \"$X\" > out.txt; exit 3"}, {"PATH=/usr/bin:/bin", "X=7"});
    });
    CHECK(code == 3);
    CHECK(slurp("out.txt") == "7\n");
}
