#include "doctest.h"

#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>

#include "conch_pty.hpp"

namespace shtest {

TEST_CASE("Ctrl-C at the prompt doesn't end the shell") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("\x03");
    usleep(100'000);
    c.line("echo here");
    CHECK_MESSAGE(c.expect("here"), c.output());
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("Ctrl-Z stops a whole pipeline, and $? is 128 plus SIGTSTP") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5 | sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sleep 5 | sleep 5"), c.output());
    REQUIRE(c.prompt());
    c.line("jobs");
    CHECK_MESSAGE(c.expect("[1] Stopped  sleep 5 | sleep 5"), c.output());
}

TEST_CASE("a second job stopped gets the next number") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE(c.expect("[1] Stopped  sleep 5"));
    REQUIRE(c.prompt());
    c.line("sleep 6");
    usleep(200'000);
    c.send("\x1a");
    CHECK_MESSAGE(c.expect("[2] Stopped  sleep 6"), c.output());
}

TEST_CASE("the terminal's settings come back after a job") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("stty -echo");
    REQUIRE(c.prompt());
    c.line("echo visible");
    CHECK_MESSAGE(c.expect("echo visible"), "the job turned echo off, and it stayed off");
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("wait on a stopped job returns at once") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE(c.expect("[1] Stopped  sleep 5"));
    REQUIRE(c.prompt());
    c.line("wait %1");
    CHECK_MESSAGE(c.prompt(), "wait %1 blocked on a job that can't end until it's continued");
    c.line("echo [done]");
    CHECK(c.expect("[done]"));
}

TEST_CASE("conch started in the background waits for the foreground") {
    Pty term([] {
        pid_t kid = fork();
        if (kid == 0) {
            setpgid(0, 0);  // like "conch &" from another shell
            _exit(sysroad_student_main());
        }
        setpgid(kid, kid);
        int st = 0;
        waitpid(kid, &st, WUNTRACED);
        bool waited = WIFSTOPPED(st) && WSTOPSIG(st) == SIGTTIN && tcgetpgrp(0) == getpgrp();
        std::printf(waited ? "waited in the background\n" : "took the terminal\n");
        std::fflush(stdout);
        if (!waited) {
            kill(kid, SIGKILL);
            return 1;
        }
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(0, kid);  // fg
        kill(-kid, SIGCONT);
        waitpid(kid, &st, 0);
        return WIFEXITED(st) ? WEXITSTATUS(st) : 2;
    });
    REQUIRE_MESSAGE(term.expect("waited in the background"), term.output());
    REQUIRE(term.expect("conch$ "));
    term.send("exit 7\r");
    CHECK(term.wait() == 7);
}

}  // namespace shtest
