#include "doctest.h"

#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <system_error>

#include "shell.hpp"

static std::string slurp(const char* path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static int lowest_free_fd() {
    int fd = open("/dev/null", O_RDONLY);
    close(fd);
    return fd;
}

TEST_CASE("parse_command: syntax errors") {
    CHECK_THROWS_AS(parse_command(tokenize("echo >")), SyntaxError);
    CHECK_THROWS_AS(parse_command(tokenize("echo > < x")), SyntaxError);
    CHECK_THROWS_AS(parse_command(tokenize("echo >& file")), SyntaxError);
    CHECK_THROWS_AS(parse_command(tokenize("cat <&")), SyntaxError);
}

TEST_CASE("parse_command: defaults and numbers") {
    Command c = parse_command(tokenize("x <&4 >&2 10>f"));
    REQUIRE(c.redirects.size() == 3);
    CHECK(c.redirects[0] == Redirect{Redirect::Dup, 0, "", 4});
    CHECK(c.redirects[1] == Redirect{Redirect::Dup, 1, "", 2});
    CHECK(c.redirects[2] == Redirect{Redirect::Write, 10, "f", -1});
    CHECK(parse_command(tokenize("")).argv.empty());
    CHECK(parse_command(tokenize("> f")).argv.empty());
}

TEST_CASE("apply_redirects in a child, in order") {
    pid_t pid = fork();
    if (pid == 0) {
        try {
            apply_redirects(parse_command(tokenize("x > a1.txt 2>&1")).redirects);
        } catch (...) {
            _exit(9);
        }
        write(2, "to stderr\n", 10);
        _exit(0);
    }
    CHECK(shell_status(wait_for(pid)) == 0);
    CHECK(slurp("a1.txt") == "to stderr\n");
}

TEST_CASE("apply_redirects throws, naming the file") {
    try {
        apply_redirects(parse_command(tokenize("x < no-such-file.txt")).redirects);
        FAIL("no exception");
    } catch (const std::system_error& e) {
        CHECK(std::string(e.what()).rfind("no-such-file.txt", 0) == 0);
    }
    CHECK_THROWS_AS(apply_redirects(parse_command(tokenize("x <&57")).redirects),
                    std::system_error);
}

TEST_CASE("SavedFds puts stdout back") {
    int before = lowest_free_fd();
    std::fflush(nullptr);
    {
        auto r = parse_command(tokenize("x > saved.txt")).redirects;
        SavedFds saved(r);
        apply_redirects(r);
        write(1, "inside\n", 7);
    }
    CHECK(slurp("saved.txt") == "inside\n");
    CHECK(fcntl(1, F_GETFD) != -1);
    CHECK(lowest_free_fd() == before);
}

TEST_CASE("SavedFds: an fd that was closed is closed again") {
    {
        auto r = parse_command(tokenize("x 7> seven.txt")).redirects;
        SavedFds saved(r);
        apply_redirects(r);
        CHECK(fcntl(7, F_GETFD) != -1);
    }
    CHECK(fcntl(7, F_GETFD) == -1);
}

TEST_CASE("SavedFds: its copies aren't inherited by commands") {
    auto r = parse_command(tokenize("x > inherit.txt")).redirects;
    SavedFds saved(r);
    for (int fd = 3; fd < 64; ++fd) {
        int flags = fcntl(fd, F_GETFD);
        if (flags != -1 && fd >= 10) {
            CHECK_MESSAGE((flags & FD_CLOEXEC) != 0, "a saved copy isn't close-on-exec");
        }
    }
}

TEST_CASE("SavedFds still saves stdout when fd 10 is past the limit") {
    pid_t pid = fork();
    if (pid == 0) {
        int out = open("lim-orig.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(out, 1);
        close(out);
        rlimit r{};
        getrlimit(RLIMIT_NOFILE, &r);
        r.rlim_cur = 10;  // F_DUPFD with a minimum of 10 now fails
        setrlimit(RLIMIT_NOFILE, &r);
        Shell sh;
        run_line(sh, "type cd > lim.txt");
        write(1, "back\n", 5);
        _exit(sh.last_status);
    }
    CHECK(shell_status(wait_for(pid)) == 0);
    CHECK(slurp("lim.txt") == "cd is a shell builtin\n");
    CHECK_MESSAGE(slurp("lim-orig.txt") == "back\n", "stdout wasn't put back");
}

TEST_CASE("with fd 10 out of reach, a saved copy can't stand in for a closed fd") {
    pid_t pid = fork();
    if (pid == 0) {
        int out = open("lim2-orig.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(out, 1);
        close(out);
        for (int fd = 3; fd < 10; ++fd) {
            close(fd);  // so the lowest free fd is 3
        }
        rlimit r{};
        getrlimit(RLIMIT_NOFILE, &r);
        r.rlim_cur = 10;
        setrlimit(RLIMIT_NOFILE, &r);
        Shell sh;
        run_line(sh, "pwd 1>&3 > lim2.txt");  // 3 is closed: this must fail
        write(1, "back\n", 5);
        _exit(sh.last_status);
    }
    CHECK(shell_status(wait_for(pid)) == 1);
    CHECK(access("lim2.txt", F_OK) == -1);
    CHECK(slurp("lim2-orig.txt") == "back\n");
}

TEST_CASE("a saved copy never lands on an fd the redirections name") {
    pid_t pid = fork();
    if (pid == 0) {
        close(10);
        Shell sh;
        run_line(sh, "pwd 1>&10 > out10.txt");  // 10 is closed: this must fail
        bool ok = sh.last_status == 1 && access("out10.txt", F_OK) == -1 &&
                  fcntl(10, F_GETFD) == -1;
        _exit(ok ? 0 : 1);
    }
    CHECK(shell_status(wait_for(pid)) == 0);
}
