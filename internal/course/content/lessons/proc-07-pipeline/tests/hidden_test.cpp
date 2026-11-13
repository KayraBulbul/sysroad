#include "doctest.h"

#include <fcntl.h>

#include <chrono>
#include <csignal>
#include <set>
#include <sstream>
#include <stdexcept>

#include "helpers.hpp"
#include "pipeline.hpp"

TEST_CASE("split_pipeline: edge cases") {
    CHECK(split_pipeline({}).empty());
    CHECK(split_pipeline({"cat"}) == std::vector<Command>{{"cat"}});
    CHECK(split_pipeline({"echo", "a|b"}) == std::vector<Command>{{"echo", "a|b"}});
    CHECK_THROWS_AS(split_pipeline({"|", "cat"}), std::invalid_argument);
    CHECK_THROWS_AS(split_pipeline({"cat", "|"}), std::invalid_argument);
    CHECK_THROWS_AS(split_pipeline({"cat", "|", "|", "cat"}), std::invalid_argument);
    CHECK_THROWS_AS(split_pipeline({"|"}), std::invalid_argument);
}

TEST_CASE("no commands, no statuses") {
    CHECK(run_pipeline({}).empty());
}

TEST_CASE("the first command reads our stdin") {
    write_file("in.txt", "from stdin\n");
    std::vector<Status> st;
    {
        Redirected in(0, "in.txt", O_RDONLY);
        Redirected out(1, "in-out.txt", O_WRONLY | O_CREAT | O_TRUNC);
        st = run_pipeline({{"cat"}, {"tr", "a-z", "A-Z"}});
    }
    CHECK(st.size() == 2);
    CHECK(slurp("in-out.txt") == "FROM STDIN\n");
}

TEST_CASE("forty cats in a row") {
    std::vector<Command> cmds{{"printf", "through every cat\\n"}};
    for (int i = 0; i < 40; ++i) {
        cmds.push_back({"cat"});
    }
    int r = within(5, [&] {
        Redirected out(1, "cats.txt", O_WRONLY | O_CREAT | O_TRUNC);
        auto st = run_pipeline(cmds);
        return st.size() == 41 && st.back().ok() ? 0 : 1;
    });
    CHECK_MESSAGE(r != -1, "the pipeline never finished: is a write end still open somewhere?");
    CHECK(r == 0);
    CHECK(slurp("cats.txt") == "through every cat\n");
}

TEST_CASE("the commands run at the same time") {
    auto start = std::chrono::steady_clock::now();
    auto st = run_pipeline({{"sleep", "0.4"}, {"sleep", "0.4"}, {"sleep", "0.4"}});
    CHECK(st.size() == 3);
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::milliseconds(1000));
}

TEST_CASE("yes | head -n 1: SIGPIPE stops yes, even if we ignore it") {
    auto old = ::signal(SIGPIPE, SIG_IGN);
    int r = within(5, [] {
        Redirected out(1, "yes.txt", O_WRONLY | O_CREAT | O_TRUNC);
        auto st = run_pipeline({{"yes"}, {"head", "-n", "1"}});
        bool ok = st.size() == 2 && st[0].kind == Status::Signaled && st[0].code == SIGPIPE &&
                  st[1].ok();
        return ok ? 0 : 1;
    });
    ::signal(SIGPIPE, old);
    CHECK_MESSAGE(r != -1, "yes never stopped");
    CHECK(r == 0);
    CHECK(slurp("yes.txt") == "y\n");
}

TEST_CASE("a command that isn't found: 127, and the rest carry on") {
    std::vector<Status> st;
    {
        Redirected out(1, "nf.txt", O_WRONLY | O_CREAT | O_TRUNC);
        Redirected err(2, "nf-err.txt", O_WRONLY | O_CREAT | O_TRUNC);
        st = run_pipeline({{"surely-not-a-command"}, {"sh", "-c", "cat; echo done"}});
    }
    REQUIRE(st.size() == 2);
    CHECK(st[0].code == 127);
    CHECK(st[1].ok());
    CHECK(slurp("nf.txt") == "done\n");
}

TEST_CASE("no pipe ends leak, here or into the commands") {
    int before = lowest_free_fd();
    std::set<int> inherited;
    std::stringstream probe;
    probe << "open=; for f in";
    for (int fd = 3; fd < 64; ++fd) {
        probe << ' ' << fd;
    }
    probe << "; do [ -e /dev/fd/$f ] && open=\"$open $f\"; done; echo $open > fds.txt; cat";
    std::vector<Status> st;
    {
        Redirected out(1, "probe-out.txt", O_WRONLY | O_CREAT | O_TRUNC);
        inherited = inherited_fds();
        st = run_pipeline({{"echo", "x"}, {"sh", "-c", probe.str()}, {"cat"}});
    }
    CHECK(st.size() == 3);
    CHECK(slurp("probe-out.txt") == "x\n");
    std::istringstream fds(slurp("fds.txt"));
    for (int fd; fds >> fd;) {
        std::string msg = "fd " + std::to_string(fd) + " was open in a command";
        CHECK_MESSAGE(inherited.count(fd) == 1, msg.c_str());
    }
    CHECK(lowest_free_fd() == before);
}
