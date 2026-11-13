#include "doctest.h"

#include <unistd.h>

#include <algorithm>

#include "helpers.hpp"
#include "reaper.hpp"

TEST_CASE("children are reaped by the handler") {
    Reaper reaper;
    pid_t a = reaper.start({"true"});
    pid_t b = reaper.start({"sh", "-c", "exit 3"});
    CHECK(a > 0);
    CHECK(b > 0);
    reaper.wait_all();
    auto done = reaper.finished();
    REQUIRE(done.size() == 2);
    auto find = [&](pid_t pid) {
        return std::find_if(done.begin(), done.end(), [&](auto& d) { return d.first == pid; });
    };
    REQUIRE(find(a) != done.end());
    REQUIRE(find(b) != done.end());
    CHECK(find(a)->second.ok());
    CHECK(find(b)->second.code == 3);
    CHECK(reaper.running() == 0);
}

TEST_CASE("running counts what hasn't ended") {
    Reaper reaper;
    reaper.start({"sleep", "0.3"});
    reaper.start({"true"});
    usleep(150'000);
    CHECK(reaper.running() == 1);
    reaper.wait_all();
    CHECK(reaper.running() == 0);
    CHECK(reaper.finished().size() == 2);
}
