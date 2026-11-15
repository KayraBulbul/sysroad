#include "doctest.h"

#include <sys/resource.h>
#include <unistd.h>

#include <climits>

#include "conch_run.hpp"

namespace shtest {

TEST_CASE("a pipeline that can't be started fails, says why, and runs none of it") {
    Ran r = conch("true | true | true && echo not-here\n", [] {
        rlimit lim{};
        getrlimit(RLIMIT_NOFILE, &lim);
        lim.rlim_cur = lowest_free_fd() + 2;  // room for one pipe, not two
        setrlimit(RLIMIT_NOFILE, &lim);
    });
    CHECK(r.status == 1);
    CHECK(r.out == "");
    CHECK_MESSAGE(r.err.rfind("conch: pipe:", 0) == 0, r.err);
}

TEST_CASE("a builtin in a pipeline runs in a child, so cd there doesn't move the shell") {
    char before[PATH_MAX];
    REQUIRE(getcwd(before, sizeof before) != nullptr);
    Shell sh;
    in_shell(sh, "cd / | true");
    char after[PATH_MAX];
    REQUIRE(getcwd(after, sizeof after) != nullptr);
    CHECK(std::string(after) == before);
}

TEST_CASE("skipped commands leave $? as it was") {
    Shell sh;
    CHECK(in_shell(sh, "false && echo no && echo no").status == 1);
    CHECK(in_shell(sh, "false || true || echo no").status == 0);
    CHECK(in_shell(sh, "true && false || true").status == 0);
    CHECK(in_shell(sh, "false; false || false").status == 1);
}

TEST_CASE("a pipeline's status is its last command's, even when an earlier one fails") {
    Shell sh;
    CHECK(in_shell(sh, "false | true").status == 0);
    CHECK(in_shell(sh, "true | false").status == 1);
}

TEST_CASE("lots of pipelines with few fds to spare") {
    std::string script;
    for (int i = 0; i < 100; ++i) {
        script += "echo x | cat | cat > /dev/null\n";
    }
    script += "echo survived\n";
    Ran r = conch(script, [] {
        rlimit lim{};
        getrlimit(RLIMIT_NOFILE, &lim);
        lim.rlim_cur = lowest_free_fd() + 8;
        setrlimit(RLIMIT_NOFILE, &lim);
    });
    CHECK_MESSAGE(r.out == "survived\n", (std::string("a leak of fds would run out: ") + r.err));
}

}  // namespace shtest
