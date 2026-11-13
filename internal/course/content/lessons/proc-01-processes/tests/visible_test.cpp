#include "doctest.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>

#include "self.hpp"

static bool has(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

TEST_CASE("stdin, stdout and stderr are open") {
    auto fds = open_fds();
    CHECK(has(fds, 0));
    CHECK(has(fds, 1));
    CHECK(has(fds, 2));
}

TEST_CASE("the environment has what getenv sees") {
    ::setenv("SYSROAD_TEST", "a=b", 1);
    auto env = environment();
    CHECK(env["SYSROAD_TEST"] == "a=b");
}

TEST_CASE("current_process: who am I") {
    ProcessInfo me = current_process();
    CHECK(me.pid == ::getpid());
    CHECK(me.ppid == ::getppid());
    CHECK_FALSE(me.cwd.empty());
}
