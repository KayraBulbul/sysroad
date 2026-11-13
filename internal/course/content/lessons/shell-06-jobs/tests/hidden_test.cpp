#include "doctest.h"

#include <fstream>

#include "conch_pty.hpp"

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

TEST_CASE("bg carries a stopped job on in the background") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 0.6");
    usleep(150'000);
    c.send("\x1a");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sleep 0.6"), c.output());
    REQUIRE(c.prompt());
    c.line("bg");
    CHECK_MESSAGE(c.expect("[1] sleep 0.6 &"), c.output());
    REQUIRE(c.prompt());
    usleep(800'000);
    c.line("");
    CHECK_MESSAGE(c.expect("[1] Done  sleep 0.6"), c.output());
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("a whole pipeline stops, and comes back") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5 | sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sleep 5 | sleep 5"), c.output());
    REQUIRE(c.prompt());
    c.line("fg %1");
    REQUIRE(c.expect("sleep 5 | sleep 5"));
    usleep(200'000);
    c.send("\x03");
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("jobs");
    c.line("echo end");
    REQUIRE(c.expect("end"));
    CHECK(c.output().find("Stopped", c.output().rfind("jobs")) == std::string::npos);
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("a background job that reads the terminal is stopped") {
    {
        std::ofstream("reader.sh") << "read x\necho got $x\n";
    }
    Conch c;
    REQUIRE(c.prompt());
    c.line("sh reader.sh &");
    REQUIRE(c.prompt());
    usleep(300'000);
    c.line("jobs");
    CHECK_MESSAGE(c.expect("[1] Stopped  sh reader.sh"), c.output());
    REQUIRE(c.prompt());
    c.line("fg");
    REQUIRE(c.expect("sh reader.sh"));
    usleep(200'000);
    c.line("typed");
    CHECK_MESSAGE(c.expect("got typed"), c.output());
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("exit warns once about stopped jobs") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE(c.expect("Stopped"));
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK_MESSAGE(c.expect("there are stopped jobs"), c.output());
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK(c.wait() == 1);  // $? from the first exit, which refused
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
    term.send("exit 7\n");
    CHECK(term.wait() == 7);
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
    c.line("fg");
    REQUIRE(c.expect("sleep 5"));
    usleep(200'000);
    c.send("\x03");
    REQUIRE(c.prompt());
    c.line("exit 0");
    CHECK(c.wait() == 0);
}

TEST_CASE("fg gives a stopped job back its own terminal settings") {
    {
        std::ofstream("modes.sh") << "stty -echo\n"
                                     "kill -TSTP $$\n"
                                     "if stty -a | grep -qw -- -echo; then echo kept-modes; "
                                     "else echo lost-modes; fi\n"
                                     "stty echo\n";
    }
    Conch c;
    REQUIRE(c.prompt());
    c.line("sh modes.sh");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sh modes.sh"), c.output());
    REQUIRE(c.prompt());
    c.line("fg");
    CHECK_MESSAGE(c.expect("kept-modes", 3000), c.output());
    REQUIRE(c.prompt());
    c.line("exit 0");
    CHECK(c.wait() == 0);
}
