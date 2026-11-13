#include "doctest.h"

#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

#include "status.hpp"

static std::string slurp(const char* path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

TEST_CASE("a command that can't be found exits 127") {
    Status s = wait_for(start({"surely-not-a-command-anywhere"}));
    CHECK(s.kind == Status::Exited);
    CHECK(s.code == 127);
}

TEST_CASE("arguments reach the command") {
    Status s = wait_for(start({"sh", "-c", "exit $1", "sh", "42"}));
    CHECK(s.code == 42);
}

TEST_CASE("SIGKILL and SIGINT") {
    Status k = wait_for(start({"sh", "-c", "kill -KILL $$"}));
    CHECK(k.kind == Status::Signaled);
    CHECK(k.code == SIGKILL);
    CHECK(describe(k) == "killed by SIGKILL");
    CHECK(shell_status(k) == 137);
    Status i = wait_for(start({"sh", "-c", "kill -INT $$"}));
    CHECK(describe(i) == "killed by SIGINT");
}

TEST_CASE("decode: a status from waitpid") {
    pid_t pid = ::fork();
    if (pid == 0) {
        ::_exit(200);
    }
    int wstatus = 0;
    ::waitpid(pid, &wstatus, 0);
    Status s = decode(wstatus);
    CHECK(s.kind == Status::Exited);
    CHECK(s.code == 200);
    CHECK_FALSE(s.core_dumped);
}

TEST_CASE("describe a core dump") {
    Status s;
    s.kind = Status::Signaled;
    s.code = SIGSEGV;
    s.core_dumped = true;
    CHECK(describe(s) == "killed by SIGSEGV (core dumped)");
    CHECK(shell_status(s) == 128 + SIGSEGV);
}

TEST_CASE("signal names") {
    CHECK(signal_name(SIGHUP) == "SIGHUP");
    CHECK(signal_name(SIGQUIT) == "SIGQUIT");
    CHECK(signal_name(SIGABRT) == "SIGABRT");
    CHECK(signal_name(SIGSEGV) == "SIGSEGV");
    CHECK(signal_name(SIGPIPE) == "SIGPIPE");
    CHECK(signal_name(SIGUSR1) == "SIGUSR1");
    CHECK(signal_name(SIGCHLD) == "SIGCHLD");
    CHECK(signal_name(SIGTSTP) == "SIGTSTP");
    CHECK(signal_name(0) == "signal 0");
    CHECK(signal_name(99) == "signal 99");
}

TEST_CASE("start flushes buffered output first") {
    std::FILE* f = std::fopen("buffered.txt", "w");
    REQUIRE(f != nullptr);
    std::fputs("once\n", f);
    Status s = wait_for(start({"true"}));
    std::fclose(f);
    CHECK(s.ok());
    CHECK(slurp("buffered.txt") == "once\n");
}

TEST_CASE("wait_for throws for a process that isn't our child") {
    CHECK_THROWS_AS(wait_for(::getpid()), std::system_error);
}

static void on_alarm(int) {}

TEST_CASE("wait_for carries on when a signal interrupts it") {
    struct sigaction sa {};
    sa.sa_handler = on_alarm;
    sa.sa_flags = 0;  // no SA_RESTART: waitpid fails with EINTR
    sigemptyset(&sa.sa_mask);
    struct sigaction old {};
    ::sigaction(SIGALRM, &sa, &old);
    itimerval t{};
    t.it_value.tv_usec = 100'000;
    ::setitimer(ITIMER_REAL, &t, nullptr);
    Status s;
    CHECK_NOTHROW(s = wait_for(start({"sleep", "0.4"})));
    ::sigaction(SIGALRM, &old, nullptr);
    CHECK(s.ok());
}

TEST_CASE("reap_finished collects the finished children only") {
    pid_t slow = start({"sleep", "2"});
    pid_t a = start({"true"});
    pid_t b = start({"sh", "-c", "exit 4"});
    ::usleep(400'000);

    auto start_time = std::chrono::steady_clock::now();
    auto done = reap_finished();
    CHECK(std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(200));
    REQUIRE(done.size() == 2);
    std::sort(done.begin(), done.end(),
              [&](const auto& x, const auto& y) { return (x.first == a) > (y.first == a); });
    CHECK(done[0].first == a);
    CHECK(done[0].second.ok());
    CHECK(done[1].first == b);
    CHECK(done[1].second.code == 4);
    CHECK(::kill(a, 0) == -1);  // reaped: no zombie left
    CHECK(reap_finished().empty());

    ::kill(slow, SIGKILL);
    CHECK(wait_for(slow).code == SIGKILL);
    CHECK(reap_finished().empty());  // no children at all
}
