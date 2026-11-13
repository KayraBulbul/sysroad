#include "doctest.h"

#include <cerrno>
#include <stdexcept>
#include <string>
#include <vector>

#include "env.hpp"
#include "helpers.hpp"

using Args = std::vector<std::string>;

TEST_CASE("parse: no arguments at all") {
    EnvCommand cmd = parse_env_args({});
    CHECK_FALSE(cmd.clear);
    CHECK(cmd.unset.empty());
    CHECK(cmd.set.empty());
    CHECK(cmd.argv.empty());
}

TEST_CASE("parse: -- ends the options") {
    EnvCommand cmd = parse_env_args({"--", "-i"});
    CHECK_FALSE(cmd.clear);
    CHECK(cmd.argv == Args{"-i"});
}

TEST_CASE("parse: an empty value, and a word starting with =") {
    EnvCommand cmd = parse_env_args({"EMPTY=", "=x", "A=1"});
    REQUIRE(cmd.set.size() == 1);
    CHECK(cmd.set[0].first == "EMPTY");
    CHECK(cmd.set[0].second == "");
    CHECK(cmd.argv == Args{"=x", "A=1"});
}

TEST_CASE("parse: bad options throw") {
    CHECK_THROWS_AS(parse_env_args({"-x"}), std::invalid_argument);
    CHECK_THROWS_AS(parse_env_args({"-u"}), std::invalid_argument);
}

TEST_CASE("build_environment: unset several, set one twice") {
    EnvCommand cmd;
    cmd.unset = {"A", "C", "MISSING"};
    cmd.set = {{"B", "1"}, {"B", "2"}};
    CHECK(build_environment({"A=1", "B=0", "C=1", "D=1"}, cmd) == Args{"B=2", "D=1"});
}

TEST_CASE("build_environment: a name is up to the first =") {
    EnvCommand cmd;
    cmd.set = {{"A", "new"}};
    CHECK(build_environment({"AB=1", "A=x=y"}, cmd) == Args{"AB=1", "A=new"});
    cmd.set.clear();
    cmd.unset = {"A"};
    CHECK(build_environment({"AB=1", "A=x=y"}, cmd) == Args{"AB=1"});
}

TEST_CASE("find_in_path: skips folders and files that aren't executable") {
    ::mkdir("p1", 0755);
    ::mkdir("p2", 0755);
    ::mkdir("p3", 0755);
    ::mkdir("p1/prog", 0755);                  // a folder
    write_file("p2/prog", "#!/bin/sh\n", 0644);  // not executable
    write_file("p3/prog", "#!/bin/sh\n", 0755);
    CHECK(find_in_path("prog", "p1:p2:p3") == "p3/prog");
    CHECK(find_in_path("prog", "p1:p2") == "");
}

TEST_CASE("find_in_path: an empty entry is the current folder") {
    write_file("here", "#!/bin/sh\n", 0755);
    CHECK(find_in_path("here", "nowhere:") == "./here");
    CHECK(find_in_path("here", ":nowhere") == "./here");
    CHECK(find_in_path("here", "nowhere::elsewhere") == "./here");
}

TEST_CASE("find_in_path: the first match wins") {
    ::mkdir("q1", 0755);
    ::mkdir("q2", 0755);
    write_file("q1/same", "#!/bin/sh\n", 0755);
    write_file("q2/same", "#!/bin/sh\n", 0755);
    CHECK(find_in_path("same", "q2:q1") == "q2/same");
}

TEST_CASE("exec_with: arguments arrive one by one, spaces and all") {
    int code = in_child([] {
        exec_with({"sh", "-c", "printf '[%s]' \"$@\" > args.txt", "sh", "two words", "", "x"},
                  {"PATH=/usr/bin:/bin"});
    });
    CHECK(code == 0);
    CHECK(slurp("args.txt") == "[two words][][x]");
}

TEST_CASE("exec_with: only the environment it's given") {
    ::setenv("SYSROAD_PARENT_ONLY", "1", 1);
    int code = in_child([] {
        exec_with({"sh", "-c", "echo \"${SYSROAD_PARENT_ONLY-none}\" > env.txt"},
                  {"PATH=/usr/bin:/bin"});
    });
    CHECK(code == 0);
    CHECK(slurp("env.txt") == "none\n");
}

TEST_CASE("exec_with: uses the new environment's PATH") {
    ::mkdir("mybin", 0755);
    write_file("mybin/hello", "#!/bin/sh\necho mine > which.txt\n", 0755);
    int code = in_child([] { exec_with({"hello"}, {"PATH=/usr/bin:mybin"}); });
    CHECK(code == 0);
    CHECK(slurp("which.txt") == "mine\n");
}

TEST_CASE("exec_with: returns with ENOENT when there's nothing to run") {
    errno = 0;
    exec_with({"surely-not-a-command"}, {"PATH=/usr/bin:/bin"});
    CHECK(errno == ENOENT);
}

TEST_CASE("exec_with: found but not executable is EACCES, not ENOENT") {
    ::mkdir("noexec", 0755);
    write_file("noexec/tool", "echo hi\n", 0644);
    errno = 0;
    exec_with({"tool"}, {"PATH=noexec"});
    CHECK(errno == EACCES);
}

TEST_CASE("exec_with: with no PATH, looks in /usr/bin and /bin") {
    int code = in_child([] { exec_with({"sh", "-c", "exit 5"}, {}); });
    CHECK(code == 5);
}
