#include "doctest.h"

#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iterator>
#include <string>

#include "helpers.hpp"
#include "shell.hpp"

static int lowest_free_fd() {
    int fd = open("/dev/null", O_RDONLY);
    close(fd);
    return fd;
}

TEST_CASE("notify_jobs doesn't wait") {
    Shell sh;
    {
        Captured cap;
        run_line(sh, "sleep 0.5 &");
    }
    auto start = std::chrono::steady_clock::now();
    {
        Captured cap;
        notify_jobs(sh);
    }
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::milliseconds(100));
    CHECK(sh.jobs.size() == 1);
    {
        Captured cap;
        run_line(sh, "wait");
    }
}

TEST_CASE("a pipeline is done when every command is") {
    Shell sh;
    {
        Captured cap;
        run_line(sh, "true | sleep 0.4 &");
        usleep(150'000);
        notify_jobs(sh);
    }
    CHECK(sh.jobs.size() == 1);  // true ended, sleep hasn't
    {
        Captured cap;
        usleep(400'000);
        notify_jobs(sh);
    }
    CHECK(Captured::out() == "[1] Done  true | sleep 0.4\n");
    CHECK(sh.jobs.empty());
}

TEST_CASE("Done, Exit and a signal") {
    Shell sh;
    {
        Captured cap;
        run_line(sh, "true &");
        run_line(sh, "false &");
        run_line(sh, "sleep 5 &");
        kill(sh.jobs[2].pids[0], SIGKILL);
        usleep(300'000);
        notify_jobs(sh);
    }
    CHECK(Captured::out() == "[1] Done  true\n[2] Exit 1  false\n[3] SIGKILL  sleep 5\n");
}

TEST_CASE("a new job's number is one more than the highest") {
    Shell sh;
    {
        Captured cap;
        run_line(sh, "sleep 0.3 &");
        run_line(sh, "true &");
        usleep(150'000);
        notify_jobs(sh);  // [2] is gone, [1] isn't
        run_line(sh, "true &");
    }
    REQUIRE(sh.jobs.size() == 2);
    CHECK(sh.jobs[1].number == 2);
    {
        Captured cap;
        run_line(sh, "wait");
    }
}

TEST_CASE("wait %n: that job's status, and it leaves the table") {
    Shell sh;
    {
        Captured cap;
        run_line(sh, "sleep 0.2 &");
        run_line(sh, "false &");
        run_line(sh, "wait %2");
    }
    CHECK(sh.last_status == 1);
    REQUIRE(sh.jobs.size() == 1);
    CHECK(sh.jobs[0].number == 1);
    {
        Captured cap;
        run_line(sh, "wait %1");
    }
    CHECK(sh.last_status == 0);
    CHECK(sh.jobs.empty());
}

TEST_CASE("wait with no jobs, and for one that isn't there") {
    Shell sh;
    {
        Captured cap;
        run_line(sh, "wait");
        CHECK(sh.last_status == 0);
        run_line(sh, "wait %3");
    }
    CHECK(sh.last_status == 127);
}

TEST_CASE("no zombies once a job is reported") {
    Shell sh;
    pid_t pid;
    {
        Captured cap;
        run_line(sh, "true &");
        pid = sh.jobs[0].pids[0];
        usleep(200'000);
        notify_jobs(sh);
    }
    CHECK(waitpid(pid, nullptr, WNOHANG) == -1);  // already reaped
}

TEST_CASE("a pipeline that can't be started fails, and says why") {
    pid_t pid = fork();
    if (pid == 0) {
        int out = open("pf-out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        int err = open("pf-err.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(out, 1);
        dup2(err, 2);
        close(out);
        close(err);
        rlimit r{};
        getrlimit(RLIMIT_NOFILE, &r);
        r.rlim_cur = lowest_free_fd() + 2;  // room for one pipe, not two
        setrlimit(RLIMIT_NOFILE, &r);
        Shell sh;
        run_line(sh, "true | true | true && echo not-here");
        _exit(sh.last_status);
    }
    CHECK(shell_status(wait_for(pid)) == 1);
    std::ifstream o("pf-out.txt"), e("pf-err.txt");
    std::string out((std::istreambuf_iterator<char>(o)), std::istreambuf_iterator<char>());
    std::string err((std::istreambuf_iterator<char>(e)), std::istreambuf_iterator<char>());
    CHECK(out.empty());
    CHECK_MESSAGE(err.find("pipe") != std::string::npos, err);
}
