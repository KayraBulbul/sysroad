#include "doctest.h"

#include <sys/wait.h>

#include <chrono>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("Done, Exit N and a signal's name, in the jobs' order") {
    // Each lasts long enough that the next one starts while it runs.
    write_file("ok.sh", "sleep 0.6\n");
    write_file("fail.sh", "sleep 0.6; exit 1\n");
    write_file("term.sh", "sleep 0.6; kill -TERM $$\n");
    Ran r = conch("sh ok.sh &\nsh fail.sh &\nsh term.sh &\nsleep 1.4\necho next\n");
    CHECK(r.out == "[1] Done  sh ok.sh\n[2] Exit 1  sh fail.sh\n[3] SIGTERM  sh term.sh\nnext\n");
}

TEST_CASE("a pipeline is done when every command in it is") {
    // true ends at once; sleep 2 runs on, so the job isn't done. Nobody
    // waits for it: conch ends, and the sleep with it soon after.
    Ran r = conch("true | sleep 2 &\nsleep 0.6\njobs\necho end\n");
    CHECK(r.out == "[1] Running  true | sleep 2\nend\n");
}

TEST_CASE("a reported job leaves no zombie") {
    Shell sh;
    Ran started = in_shell(sh, "true &");
    pid_t pid = std::stoi(started.err.substr(started.err.find(' ') + 1));
    std::string out;
    for (int i = 0; i < 60 && out.empty(); ++i) {  // until it's reported, up to 3 s
        usleep(50'000);
        out = in_shell(sh, "jobs").out;
    }
    CHECK(out == "[1] Done  true\n");
    CHECK_MESSAGE(waitpid(pid, nullptr, WNOHANG) == -1, "the job's process wasn't reaped");
}

TEST_CASE("wait %n: that job's status; and a job that isn't there") {
    write_file("exit3.sh", "exit 3\n");
    Shell sh;
    in_shell(sh, "sleep 1 &");
    in_shell(sh, "sh exit3.sh &");
    CHECK(in_shell(sh, "wait %2").status == 3);
    CHECK(in_shell(sh, "jobs").out == "[1] Running  sleep 1\n");
    CHECK(in_shell(sh, "wait %1").status == 0);
    Ran missing = in_shell(sh, "wait %4");
    CHECK(missing.status == 127);
    CHECK(missing.err == "conch: wait: %4: no such job\n");
}

TEST_CASE("wait with no jobs returns at once") {
    Shell sh;
    auto start = std::chrono::steady_clock::now();
    CHECK(in_shell(sh, "wait").status == 0);
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::milliseconds(1000));
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
