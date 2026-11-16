#include "doctest.h"

#include <fcntl.h>
#include <sys/resource.h>
#include <unistd.h>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("an fd that was closed before the builtin is closed again after it") {
    Shell sh;
    REQUIRE(fcntl(57, F_GETFD) == -1);
    in_shell(sh, "type cd 57> fifty-seven.txt");
    CHECK_MESSAGE(fcntl(57, F_GETFD) == -1, "fd 57 is still open after the builtin");
}

TEST_CASE("several redirections of the same fd are all undone") {
    Shell sh;
    int before = lowest_free_fd();
    in_shell(sh, "type cd > a.txt > b.txt 2>&1");
    CHECK(read_file("b.txt") == "cd is a shell builtin\n");
    CHECK(in_shell(sh, "type pwd").out == "pwd is a shell builtin\n");
    CHECK(lowest_free_fd() == before);
}

TEST_CASE("with a limit of 10 open files, a builtin can still be redirected") {
    Ran r = conch("type cd > lim.txt\necho back\n", [] {
        rlimit lim{};
        getrlimit(RLIMIT_NOFILE, &lim);
        lim.rlim_cur = 10;  // no room for fds at 10 or above
        setrlimit(RLIMIT_NOFILE, &lim);
    });
    CHECK(read_file("lim.txt") == "cd is a shell builtin\n");
    CHECK_MESSAGE(r.out == "back\n", "stdout wasn't put back");
}

TEST_CASE("with fd 10 out of reach, a saved copy can't stand in for a closed fd") {
    Ran r = conch("pwd 1>&3 > lim2.txt\necho back\n", [] {
        for (int fd = 3; fd < 10; ++fd) {
            close(fd);  // so the lowest free fd is 3
        }
        rlimit lim{};
        getrlimit(RLIMIT_NOFILE, &lim);
        lim.rlim_cur = 10;
        setrlimit(RLIMIT_NOFILE, &lim);
    });
    CHECK(r.err == "conch: 3: Bad file descriptor\n");
    CHECK(access("lim2.txt", F_OK) == -1);
    CHECK(r.out == "back\n");
}

TEST_CASE("a saved copy never lands on an fd the redirections name") {
    Ran r = conch("pwd 1>&10 > out10.txt\n", [] { close(10); });
    CHECK(r.status == 1);
    CHECK(access("out10.txt", F_OK) == -1);
}

}  // namespace shtest
