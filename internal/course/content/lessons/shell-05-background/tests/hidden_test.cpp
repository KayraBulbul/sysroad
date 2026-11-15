#include "doctest.h"

#include <sys/wait.h>

#include <chrono>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("Done, Exit N and a signal's name, in the jobs' order") {
    // Each lasts long enough that the next one starts while it runs.
    write_file("ok.sh", "sleep 0.3\n");
    write_file("fail.sh", "sleep 0.3; exit 1\n");
    write_file("term.sh", "sleep 0.3; kill -TERM $$\n");
    Ran r = conch("sh ok.sh &\nsh fail.sh &\nsh term.sh &\nsleep 0.8\necho next\n");
    CHECK(r.out == "[1] Done  sh ok.sh\n[2] Exit 1  sh fail.sh\n[3] SIGTERM  sh term.sh\nnext\n");
}

TEST_CASE("a pipeline is done when every command in it is") {
    Ran r = conch("true | sleep 0.4 &\nsleep 0.15\njobs\nwait\njobs\necho end\n");
    CHECK(r.out == "[1] Running  true | sleep 0.4\nend\n");
}

TEST_CASE("a reported job leaves no zombie") {
    Shell sh;
    Ran started = in_shell(sh, "true &");
    pid_t pid = std::stoi(started.err.substr(started.err.find(' ') + 1));
    usleep(200'000);
    CHECK(in_shell(sh, "jobs").out == "[1] Done  true\n");
    CHECK_MESSAGE(waitpid(pid, nullptr, WNOHANG) == -1, "the job's process wasn't reaped");
}

TEST_CASE("wait %n: that job's status; and a job that isn't there") {
    write_file("exit3.sh", "sleep 0.1; exit 3\n");
    Shell sh;
    in_shell(sh, "sleep 0.2 &");
    in_shell(sh, "sh exit3.sh &");
    CHECK(in_shell(sh, "wait %2").status == 3);
    CHECK(in_shell(sh, "jobs").out == "[1] Running  sleep 0.2\n");
    CHECK(in_shell(sh, "wait %1").status == 0);
    Ran missing = in_shell(sh, "wait %4");
    CHECK(missing.status == 127);
    CHECK(missing.err == "conch: wait: %4: no such job\n");
}

TEST_CASE("wait with no jobs returns at once") {
    Shell sh;
    auto start = std::chrono::steady_clock::now();
    CHECK(in_shell(sh, "wait").status == 0);
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::milliseconds(200));
}

TEST_CASE("a builtin in the background doesn't run in the shell") {
    Shell sh;
    in_shell(sh, "cd / &");
    in_shell(sh, "wait");
    char now[4096];
    REQUIRE(getcwd(now, sizeof now) != nullptr);
    CHECK(std::string(now) != "/");
}

}  // namespace shtest
