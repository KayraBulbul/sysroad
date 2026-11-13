#include "doctest.h"

#include <termios.h>

#include <cstdio>

#include "jobctl.hpp"
#include "pty.hpp"

static void say(const char* fmt, const std::string& a = "", const std::string& b = "") {
    std::printf(fmt, a.c_str(), b.c_str());
    std::fflush(stdout);
}

TEST_CASE("init_job_control ignores the job signals and takes the terminal") {
    Pty term([] {
        init_job_control(0);
        for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU}) {
            struct sigaction sa{};
            sigaction(sig, nullptr, &sa);
            if (sa.sa_handler != SIG_IGN) {
                say("%s isn't ignored\n", signal_name(sig));
                return 1;
            }
        }
        return tcgetpgrp(0) == getpgrp() ? 0 : 2;
    });
    int r = term.wait();
    CHECK_MESSAGE(r == 0, term.output());
}

TEST_CASE("a stopped job carries on in the foreground") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"sleep", "5"}}, true, 0);
        say("launched\n");
        wait_foreground(job, 0);
        say("stopped\n");
        continue_job(job, 0, true);
        say("then %s\n", describe(job.statuses[0]));
        return job.done() ? 0 : 1;
    });
    REQUIRE(term.expect("launched"));
    usleep(100'000);
    term.send("\x1a");
    REQUIRE_MESSAGE(term.expect("stopped"), term.output());
    usleep(200'000);
    term.send("\x03");  // reaches the job only if it has the terminal again
    CHECK_MESSAGE(term.expect("then killed by SIGINT"), term.output());
    CHECK(term.wait() == 0);
}

TEST_CASE("a stopped job carries on in the background") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"sh", "-c", "sleep 0.5; echo background done"}}, true, 0);
        say("launched\n");
        wait_foreground(job, 0);
        if (!job.stopped) {
            return 1;
        }
        continue_job(job, 0, false);
        say("carrying on, terminal is %s\n", tcgetpgrp(0) == getpgrp() ? "mine" : "not mine");
        wait_for_job(job);
        say("finally %s\n", describe(job.statuses[0]));
        return 0;
    });
    REQUIRE(term.expect("launched"));
    usleep(100'000);
    term.send("\x1a");
    CHECK_MESSAGE(term.expect("carrying on, terminal is mine"), term.output());
    CHECK_MESSAGE(term.expect("background done"), term.output());
    CHECK_MESSAGE(term.expect("finally exited 0"), term.output());
    CHECK(term.wait() == 0);
}

TEST_CASE("a background job that reads the terminal is stopped by SIGTTIN") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"cat"}}, false, 0);
        wait_for_job(job);
        say("stopped=%s by %s\n", job.stopped ? "yes" : "no", signal_name(job.stop_signal));
        kill(-job.pgid, SIGKILL);
        return 0;
    });
    CHECK_MESSAGE(term.expect("stopped=yes by SIGTTIN"), term.output());
    CHECK(term.wait() == 0);
}

TEST_CASE("Ctrl-Z stops a whole pipeline, and Ctrl-C ends all of it") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"sleep", "5"}, {"sleep", "5"}, {"sleep", "5"}}, true, 0);
        say("launched\n");
        wait_foreground(job, 0);
        say(job.stopped ? "stopped\n" : "not stopped\n");
        continue_job(job, 0, true);
        int sigints = 0;
        for (const Status& s : job.statuses) {
            sigints += s.kind == Status::Signaled && s.code == SIGINT;
        }
        say(job.done() && sigints == 3 ? "all three ended\n" : "not all ended\n");
        return 0;
    });
    REQUIRE(term.expect("launched"));
    usleep(150'000);
    term.send("\x1a");
    REQUIRE_MESSAGE(term.expect("stopped"), term.output());
    usleep(150'000);
    term.send("\x03");
    CHECK_MESSAGE(term.expect("all three ended"), term.output());
    CHECK(term.wait() == 0);
}

TEST_CASE("statuses of a finished pipeline, in order") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"sh", "-c", "exit 1"}, {"sh", "-c", "cat; exit 2"}}, true, 0);
        wait_foreground(job, 0);
        bool ok = job.done() && !job.stopped && job.statuses[0].code == 1 &&
                  job.statuses[1].code == 2 && tcgetpgrp(0) == getpgrp();
        return ok ? 0 : 1;
    });
    CHECK_MESSAGE(term.wait() == 0, term.output());
}

TEST_CASE("the shell's terminal modes come back after the job") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"stty", "-echo", "-icanon"}}, true, 0);
        wait_foreground(job, 0);
        termios t{};
        tcgetattr(0, &t);
        return (t.c_lflag & ECHO) && (t.c_lflag & ICANON) ? 0 : 1;
    });
    CHECK_MESSAGE(term.wait() == 0, "the job turned echo off, and it stayed off");
}

TEST_CASE("a command that isn't found") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"surely-not-a-command"}}, true, 0);
        wait_foreground(job, 0);
        return job.done() && job.statuses[0].code == 127 && tcgetpgrp(0) == getpgrp() ? 0 : 1;
    });
    CHECK_MESSAGE(term.wait() == 0, term.output());
}

TEST_CASE("started in the background, init_job_control waits instead of taking the terminal") {
    Pty term([] {
        pid_t kid = fork();
        if (kid == 0) {
            setpgid(0, 0);  // a background job of ours
            init_job_control(0);
            _exit(tcgetpgrp(0) == getpgrp() ? 0 : 1);
        }
        setpgid(kid, kid);
        int st = 0;
        waitpid(kid, &st, WUNTRACED);
        bool waited = WIFSTOPPED(st) && WSTOPSIG(st) == SIGTTIN;
        bool kept = tcgetpgrp(0) == getpgrp();
        if (!waited) {
            say("it didn't wait: %s\n", kept ? "" : "and it took the terminal");
            return 1;
        }
        // Now bring it to the foreground, as fg would.
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(0, kid);
        kill(-kid, SIGCONT);
        waitpid(kid, &st, 0);
        tcsetpgrp(0, getpgrp());
        return kept && WIFEXITED(st) && WEXITSTATUS(st) == 0 ? 0 : 2;
    });
    CHECK_MESSAGE(term.wait() == 0, term.output());
}
