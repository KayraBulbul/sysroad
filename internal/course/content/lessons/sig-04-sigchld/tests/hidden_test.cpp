#include "doctest.h"

#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>

#include "helpers.hpp"
#include "reaper.hpp"

TEST_CASE("a burst of children: one SIGCHLD can stand for many") {
    int r = within(8, [] {
        Reaper reaper;
        std::map<pid_t, int> started;
        for (int i = 0; i < 100; ++i) {
            started[reaper.start({"sh", "-c", "exit " + std::to_string(i % 7)})] = i % 7;
        }
        reaper.wait_all();
        auto done = reaper.finished();
        if (done.size() != 100) {
            return 2;
        }
        for (const auto& [pid, status] : done) {
            if (!started.count(pid) || status.code != started[pid]) {
                return 3;
            }
            started.erase(pid);  // each exactly once
        }
        return reaper.running() == 0 ? 0 : 4;
    });
    CHECK_MESSAGE(r != -1, "wait_all never returned: a child was never counted as ended");
    CHECK_MESSAGE(r != 2, "not every child was reaped exactly once");
    CHECK(r == 0);
}

TEST_CASE("no zombies are left, even without asking") {
    int r = within(4, [] {
        Reaper reaper;
        pid_t pid = reaper.start({"true"});
        usleep(300'000);  // never call finished() or wait_all()
        // A zombie still answers kill(pid, 0); a reaped child doesn't.
        return ::kill(pid, 0) == -1 && errno == ESRCH ? 0 : 1;
    });
    CHECK(r == 0);
}

TEST_CASE("the handler leaves errno alone") {
    int r = within(4, [] {
        Reaper reaper;
        reaper.start({"true"});
        errno = EDOM;
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
        while (std::chrono::steady_clock::now() < until) {
            // spin without system calls, so only the handler could touch errno
        }
        return errno == EDOM ? 0 : 1;
    });
    CHECK(r == 0);
}

TEST_CASE("finished hands each child over once") {
    Reaper reaper;
    reaper.start({"true"});
    reaper.wait_all();
    CHECK(reaper.finished().size() == 1);
    CHECK(reaper.finished().empty());
}

TEST_CASE("wait_all sleeps instead of spinning") {
    int r = within(4, [] {
        Reaper reaper;
        reaper.start({"sleep", "0.5"});
        reaper.wait_all();
        rusage ru{};
        getrusage(RUSAGE_SELF, &ru);
        long ms = (ru.ru_utime.tv_sec + ru.ru_stime.tv_sec) * 1000 +
                  (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) / 1000;
        return ms < 200 ? 0 : 1;
    });
    CHECK(r == 0);
}

TEST_CASE("wait_all with nothing running returns at once") {
    int r = within(2, [] {
        Reaper reaper;
        reaper.wait_all();
        return 0;
    });
    CHECK(r == 0);
}

TEST_CASE("the child gets the caller's mask, not one with SIGCHLD blocked") {
    Reaper reaper;
    pid_t pid = reaper.start({"sleep", "1"});
    usleep(200'000);
    std::string cmd = "ps -o blocked= -p " + std::to_string(pid);
    std::FILE* ps = ::popen(cmd.c_str(), "r");
    REQUIRE(ps != nullptr);
    char buf[128] = {};
    size_t n = std::fread(buf, 1, sizeof buf - 1, ps);
    ::pclose(ps);
    buf[n] = '\0';
    unsigned long long mask = std::strtoull(buf, nullptr, 16);
    CHECK_MESSAGE((mask & (1ULL << (SIGCHLD - 1))) == 0,
                  "the program started with SIGCHLD blocked; put the old mask back before exec");
    ::kill(pid, SIGKILL);
    reaper.wait_all();
}

TEST_CASE("the destructor puts the old action back") {
    struct sigaction before{};
    sigaction(SIGCHLD, nullptr, &before);
    {
        Reaper reaper;
        struct sigaction during{};
        sigaction(SIGCHLD, nullptr, &during);
        CHECK(during.sa_handler != before.sa_handler);
        CHECK((during.sa_flags & SA_RESTART) != 0);
    }
    struct sigaction after{};
    sigaction(SIGCHLD, nullptr, &after);
    CHECK(after.sa_handler == before.sa_handler);
}

TEST_CASE("the caller's mask is unchanged afterwards") {
    Reaper reaper;
    reaper.start({"true"});
    reaper.wait_all();
    reaper.finished();
    sigset_t now;
    sigprocmask(SIG_BLOCK, nullptr, &now);
    CHECK(sigismember(&now, SIGCHLD) == 0);
}
