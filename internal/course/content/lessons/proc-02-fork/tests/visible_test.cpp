#include "doctest.h"

#include <unistd.h>

#include <vector>

#include "spawn.hpp"

TEST_CASE("the child's exit code reaches the parent") {
    pid_t pid = spawn([] { return 42; });
    CHECK(pid > 0);
    CHECK(pid != ::getpid());
    CHECK(wait_exit_code(pid) == 42);
}

TEST_CASE("the child changes its own copy") {
    int x = 1;
    pid_t pid = spawn([&x] {
        x = 2;
        return x;
    });
    CHECK(wait_exit_code(pid) == 2);
    CHECK(x == 1);
}

TEST_CASE("run_all") {
    auto codes = run_all(4, [](int i) { return i + 10; });
    CHECK(codes == std::vector<int>{10, 11, 12, 13});
}
