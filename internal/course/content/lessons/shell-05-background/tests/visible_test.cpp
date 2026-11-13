#include "doctest.h"

#include <chrono>

#include "helpers.hpp"
#include "shell.hpp"

TEST_CASE("a background job goes in the table, and the shell carries on") {
    Shell sh;
    auto start = std::chrono::steady_clock::now();
    {
        Captured cap;
        run_line(sh, "sleep 0.3 &");
    }
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::milliseconds(200));
    CHECK(sh.last_status == 0);
    REQUIRE(sh.jobs.size() == 1);
    CHECK(sh.jobs[0].number == 1);
    CHECK(sh.jobs[0].text == "sleep 0.3");
    CHECK(Captured::err() == "[1] " + std::to_string(sh.jobs[0].pids.back()) + "\n");
    {
        Captured cap;
        run_line(sh, "wait");
    }
    CHECK(sh.jobs.empty());
}

TEST_CASE("notify_jobs reports a finished job once") {
    Shell sh;
    {
        Captured cap;
        run_line(sh, "true &");
        usleep(300'000);
        notify_jobs(sh);
        notify_jobs(sh);
    }
    CHECK(Captured::out() == "[1] Done  true\n");
    CHECK(sh.jobs.empty());
}
