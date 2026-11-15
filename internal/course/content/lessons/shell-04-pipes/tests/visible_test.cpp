#include "doctest.h"

#include <chrono>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("the commands of a pipeline run at the same time") {
    Shell sh;
    auto start = std::chrono::steady_clock::now();
    in_shell(sh, "sleep 0.6 | sleep 0.6 | sleep 0.6");
    auto took = std::chrono::steady_clock::now() - start;
    // At once, about 0.6 s; one after another, 1.8 s.
    CHECK_MESSAGE(took < std::chrono::milliseconds(1200), "three 0.6 s sleeps took over 1.2 s: one after another?");
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
