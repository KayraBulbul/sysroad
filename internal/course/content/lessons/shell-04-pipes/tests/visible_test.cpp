#include "doctest.h"

#include <chrono>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("the commands of a pipeline run at the same time") {
    Shell sh;
    auto start = std::chrono::steady_clock::now();
    in_shell(sh, "sleep 0.4 | sleep 0.4 | sleep 0.4");
    auto took = std::chrono::steady_clock::now() - start;
    CHECK_MESSAGE(took < std::chrono::milliseconds(1000), "three 0.4 s sleeps took over a second: one after another?");
}

TEST_CASE("no pipe ends stay open in the shell") {
    Shell sh;
    int before = lowest_free_fd();
    for (int i = 0; i < 20; ++i) {
        in_shell(sh, "true | true | true");
    }
    CHECK_MESSAGE(lowest_free_fd() == before, "the shell has fds open after its pipelines ended");
}

}  // namespace shtest
