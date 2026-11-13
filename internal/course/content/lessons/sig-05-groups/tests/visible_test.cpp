#include "doctest.h"

#include <unistd.h>

#include <csignal>

#include "job.hpp"

TEST_CASE("a job gets a process group of its own") {
    Job job = launch({{"sleep", "5"}, {"sleep", "5"}, {"sleep", "5"}});
    REQUIRE(job.pids.size() == 3);
    CHECK(job.pgid == job.pids[0]);
    CHECK(job.pgid != getpgrp());
    for (pid_t pid : job.pids) {
        CHECK(getpgid(pid) == job.pgid);
    }
    signal_job(job, SIGTERM);
    for (const Status& s : wait_job(job)) {
        CHECK(s.kind == Status::Signaled);
        CHECK(s.code == SIGTERM);
    }
}

TEST_CASE("a detached child leads a session of its own") {
    pid_t pid = start_detached({"sleep", "0.3"});
    pid_t sid = -1;
    for (int i = 0; i < 100 && sid != pid; ++i) {
        usleep(10'000);
        sid = getsid(pid);
    }
    CHECK(sid == pid);
    CHECK(getpgid(pid) == pid);
    CHECK(getsid(0) != pid);
    CHECK(wait_for(pid).ok());
}
