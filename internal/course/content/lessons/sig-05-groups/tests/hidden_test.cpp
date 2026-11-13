#include "doctest.h"

#include <fcntl.h>
#include <sys/resource.h>
#include <unistd.h>

#include <csignal>
#include <system_error>

#include "helpers.hpp"
#include "job.hpp"

TEST_CASE("every process is in the group by the time launch returns") {
    for (int i = 0; i < 25; ++i) {
        Job job = launch({{"sleep", "5"}, {"sleep", "5"}});
        REQUIRE(job.pids.size() == 2);
        bool in = getpgid(job.pids[0]) == job.pgid && getpgid(job.pids[1]) == job.pgid;
        signal_job(job, SIGKILL);
        wait_job(job);
        REQUIRE_MESSAGE(in, "a process wasn't in the group yet: the parent must call setpgid too");
    }
}

TEST_CASE("the pipes still connect the commands") {
    std::vector<Status> st;
    {
        Redirected out(1, "job-out.txt", O_WRONLY | O_CREAT | O_TRUNC);
        Job job = launch({{"printf", "b\\na\\n"}, {"sort"}, {"tr", "a-z", "A-Z"}});
        st = wait_job(job);
    }
    REQUIRE(st.size() == 3);
    for (const Status& s : st) {
        CHECK(s.ok());
    }
    CHECK(slurp("job-out.txt") == "A\nB\n");
}

TEST_CASE("one kill reaches the whole group, and nothing else") {
    Job a = launch({{"sleep", "5"}, {"sleep", "5"}});
    Job b = launch({{"sleep", "0.3"}});
    signal_job(a, SIGINT);
    auto sa = wait_job(a);
    auto sb = wait_job(b);
    CHECK(sa[0].code == SIGINT);
    CHECK(sa[1].code == SIGINT);
    CHECK(sb[0].ok());
}

TEST_CASE("signal_job throws once the job is gone") {
    Job job = launch({{"true"}});
    wait_job(job);
    CHECK_THROWS_AS(signal_job(job, SIGTERM), std::system_error);
}

TEST_CASE("launch doesn't wait") {
    Job job = launch({{"sleep", "5"}});
    CHECK(getpgid(job.pids[0]) == job.pgid);  // still running
    signal_job(job, SIGKILL);
    CHECK(wait_job(job)[0].code == SIGKILL);
}

TEST_CASE("a command that isn't found exits 127, in its group") {
    Job job;
    {
        Redirected err(2, "nf-err.txt", O_WRONLY | O_CREAT | O_TRUNC);
        job = launch({{"surely-not-a-command"}, {"cat"}});
    }
    auto st = wait_job(job);
    CHECK(st[0].code == 127);
    CHECK(st[1].ok());
}

TEST_CASE("no pipe ends leak") {
    int before = lowest_free_fd();
    for (int i = 0; i < 20; ++i) {
        wait_job(launch({{"true"}, {"true"}, {"true"}}));
    }
    CHECK(lowest_free_fd() == before);
}

TEST_CASE("start_detached: a command that isn't found exits 127") {
    Redirected err(2, "det-err.txt", O_WRONLY | O_CREAT | O_TRUNC);
    CHECK(wait_for(start_detached({"surely-not-a-command"})).code == 127);
}

TEST_CASE("a job that can't be fully started ends what it started, and throws") {
    int r = within(3, [] {
        rlimit lim{};
        getrlimit(RLIMIT_NOFILE, &lim);
        lim.rlim_cur = lowest_free_fd() + 2;  // room for one pipe, not two
        setrlimit(RLIMIT_NOFILE, &lim);
        try {
            wait_job(launch({{"sleep", "5"}, {"sleep", "5"}, {"cat"}}));
        } catch (const std::system_error&) {
            return 0;
        }
        return 1;
    });
    CHECK_MESSAGE(r != -1, "it waited for the commands it had started instead of ending them");
    CHECK(r == 0);
}
