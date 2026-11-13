#include "doctest.h"

#include <cstdio>

#include "jobctl.hpp"
#include "pty.hpp"

TEST_CASE("Ctrl-Z stops the foreground job, and the shell gets the terminal back") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"sleep", "5"}}, true, 0);
        std::printf("launched\n");
        std::fflush(stdout);
        wait_foreground(job, 0);
        std::printf("stopped=%d by %s, terminal is %s\n", job.stopped,
                    signal_name(job.stop_signal).c_str(),
                    tcgetpgrp(0) == getpgrp() ? "mine" : "not mine");
        std::fflush(stdout);
        kill(-job.pgid, SIGKILL);
        return 0;
    });
    REQUIRE(term.expect("launched"));
    usleep(100'000);
    term.send("\x1a");  // Ctrl-Z
    CHECK_MESSAGE(term.expect("stopped=1 by SIGTSTP, terminal is mine"), term.output());
    CHECK(term.wait() == 0);
}

TEST_CASE("Ctrl-C ends the job, not the shell") {
    Pty term([] {
        init_job_control(0);
        Job job = launch_job({{"sleep", "5"}}, true, 0);
        std::printf("launched\n");
        std::fflush(stdout);
        wait_foreground(job, 0);
        std::printf("job %s; shell still here\n", describe(job.statuses[0]).c_str());
        return 0;
    });
    REQUIRE(term.expect("launched"));
    usleep(100'000);
    term.send("\x03");  // Ctrl-C
    CHECK_MESSAGE(term.expect("job killed by SIGINT; shell still here"), term.output());
    CHECK(term.wait() == 0);
}
