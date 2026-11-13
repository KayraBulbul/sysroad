#include "doctest.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <string>

#include "self.hpp"

static bool has(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

TEST_CASE("a newly opened fd shows up, and goes away when closed") {
    int fd = ::open("/dev/null", O_RDONLY);
    REQUIRE(fd >= 0);
    int high = ::dup2(fd, 57);
    REQUIRE(high == 57);
    auto fds = open_fds();
    CHECK(has(fds, fd));
    CHECK(has(fds, 57));
    ::close(fd);
    ::close(57);
    fds = open_fds();
    CHECK_FALSE(has(fds, fd));
    CHECK_FALSE(has(fds, 57));
}

TEST_CASE("open_fds: sorted, and not the folder's own fd") {
    auto fds = open_fds();
    CHECK(std::is_sorted(fds.begin(), fds.end()));
    // The lowest free fd now is the one opendir would have got.
    int next = ::open("/dev/null", O_RDONLY);
    REQUIRE(next >= 0);
    ::close(next);
    CHECK_FALSE(has(fds, next));
}

TEST_CASE("open_fds doesn't leak the folder") {
    for (int i = 0; i < 300; ++i) {
        open_fds();
    }
    int next = ::open("/dev/null", O_RDONLY);
    CHECK(next < 20);
    ::close(next);
}

TEST_CASE("environment: split at the first =, and empty values") {
    ::setenv("SYSROAD_EQ", "x=y=z", 1);
    ::setenv("SYSROAD_EMPTY", "", 1);
    ::unsetenv("SYSROAD_GONE");
    auto env = environment();
    CHECK(env["SYSROAD_EQ"] == "x=y=z");
    REQUIRE(env.count("SYSROAD_EMPTY") == 1);
    CHECK(env["SYSROAD_EMPTY"] == "");
    CHECK(env.count("SYSROAD_GONE") == 0);
    CHECK(env.count("SYSROAD_EQ=x") == 0);
}

TEST_CASE("current_process follows chdir") {
    char before[PATH_MAX];
    REQUIRE(::getcwd(before, sizeof before) != nullptr);
    ::mkdir("sub", 0755);
    REQUIRE(::chdir("sub") == 0);
    ProcessInfo me = current_process();
    REQUIRE(::chdir(before) == 0);
    char real[PATH_MAX];
    REQUIRE(::realpath("sub", real) != nullptr);
    char got[PATH_MAX];
    REQUIRE(::realpath(me.cwd.c_str(), got) != nullptr);
    CHECK(std::string(got) == std::string(real));
    CHECK(has(me.fds, 2));
}
