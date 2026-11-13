#include "doctest.h"

#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include "helpers.hpp"
#include "mask.hpp"

static bool blocked(int sig) {
    sigset_t now;
    sigprocmask(SIG_BLOCK, nullptr, &now);
    return sigismember(&now, sig) == 1;
}

TEST_CASE("BlockSignals nests") {
    CHECK_FALSE(blocked(SIGUSR1));
    {
        BlockSignals outer({SIGUSR1});
        {
            BlockSignals inner({SIGUSR1, SIGUSR2});
            CHECK(blocked(SIGUSR1));
            CHECK(blocked(SIGUSR2));
            CHECK(sigismember(&inner.previous(), SIGUSR1) == 1);
            CHECK(sigismember(&inner.previous(), SIGUSR2) == 0);
        }
        CHECK(blocked(SIGUSR1));
        CHECK_FALSE(blocked(SIGUSR2));
    }
    CHECK_FALSE(blocked(SIGUSR1));
}

TEST_CASE("is_pending: nothing pending") {
    CHECK_FALSE(is_pending(SIGUSR1));
    CHECK_FALSE(is_pending(SIGUSR2));
}

TEST_CASE("await_signal: a signal that came before the call counts") {
    int r = within(2, [] {
        BlockSignals block({SIGUSR1});
        raise(SIGUSR1);
        return await_signal({SIGUSR1}) == SIGUSR1 ? 0 : 1;
    });
    CHECK_MESSAGE(r != -1, "the signal was already pending, but await_signal slept anyway");
    CHECK(r == 0);
}

TEST_CASE("await_signal: says which one came") {
    int r = within(3, [] {
        BlockSignals block({SIGUSR1, SIGUSR2});
        pid_t child = fork();
        if (child == 0) {
            usleep(100'000);
            kill(getppid(), SIGUSR2);
            _exit(0);
        }
        int sig = await_signal({SIGUSR1, SIGUSR2});
        waitpid(child, nullptr, 0);
        return sig == SIGUSR2 ? 0 : 1;
    });
    CHECK(r == 0);
}

TEST_CASE("await_signal: without blocking first, a later signal still arrives") {
    int r = within(3, [] {
        pid_t child = fork();
        if (child == 0) {
            usleep(200'000);
            kill(getppid(), SIGUSR1);
            _exit(0);
        }
        int sig = await_signal({SIGUSR1});
        waitpid(child, nullptr, 0);
        return sig == SIGUSR1 ? 0 : 1;
    });
    CHECK(r == 0);
}

TEST_CASE("await_signal puts the mask and the actions back") {
    int r = within(3, [] {
        struct sigaction before{};
        sigaction(SIGUSR1, nullptr, &before);
        BlockSignals block({SIGUSR1});
        raise(SIGUSR1);
        await_signal({SIGUSR1});
        struct sigaction after{};
        sigaction(SIGUSR1, nullptr, &after);
        if (after.sa_handler != before.sa_handler) {
            return 2;
        }
        if (!blocked(SIGUSR1)) {
            return 3;  // the caller's block must still be there
        }
        return 0;
    });
    CHECK_MESSAGE(r != 2, "SIGUSR1's old action wasn't put back");
    CHECK_MESSAGE(r != 3, "the caller's mask wasn't put back");
    CHECK(r == 0);
    CHECK_FALSE(blocked(SIGUSR2));
}

TEST_CASE("await_signal sleeps instead of spinning") {
    int r = within(3, [] {
        BlockSignals block({SIGUSR1});
        pid_t child = fork();
        if (child == 0) {
            usleep(500'000);
            kill(getppid(), SIGUSR1);
            _exit(0);
        }
        await_signal({SIGUSR1});
        waitpid(child, nullptr, 0);
        rusage ru{};
        getrusage(RUSAGE_SELF, &ru);
        long cpu_ms = (ru.ru_utime.tv_sec + ru.ru_stime.tv_sec) * 1000 +
                      (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) / 1000;
        return cpu_ms < 200 ? 0 : 1;
    });
    CHECK_MESSAGE(r != 1, "it used the CPU the whole time it waited");
    CHECK(r == 0);
}

TEST_CASE("no lost wakeups, many times over") {
    int r = within(5, [] {
        for (int i = 0; i < 60; ++i) {
            BlockSignals block({SIGUSR1});
            pid_t child = fork();
            if (child == 0) {
                kill(getppid(), SIGUSR1);
                _exit(0);
            }
            if (await_signal({SIGUSR1}) != SIGUSR1) {
                return 1;
            }
            waitpid(child, nullptr, 0);
        }
        return 0;
    });
    CHECK(r == 0);
}
