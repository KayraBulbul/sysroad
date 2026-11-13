#include "doctest.h"

#include <fcntl.h>

#include "helpers.hpp"
#include "pipeline.hpp"

using Words = std::vector<std::string>;

TEST_CASE("split_pipeline") {
    auto cmds = split_pipeline({"ls", "-l", "|", "grep", "x", "|", "wc"});
    REQUIRE(cmds.size() == 3);
    CHECK(cmds[0] == Command{"ls", "-l"});
    CHECK(cmds[1] == Command{"grep", "x"});
    CHECK(cmds[2] == Command{"wc"});
}

TEST_CASE("two commands, connected") {
    std::vector<Status> st;
    {
        Redirected out(1, "out.txt", O_WRONLY | O_CREAT | O_TRUNC);
        st = run_pipeline({{"printf", "b\\na\\n"}, {"sort"}});
    }
    REQUIRE(st.size() == 2);
    CHECK(st[0].ok());
    CHECK(st[1].ok());
    CHECK(slurp("out.txt") == "a\nb\n");
}

TEST_CASE("each command's status, in order") {
    auto st = run_pipeline({{"sh", "-c", "exit 1"}, {"sh", "-c", "cat; exit 2"},
                            {"sh", "-c", "cat >/dev/null; exit 3"}});
    REQUIRE(st.size() == 3);
    CHECK(st[0].code == 1);
    CHECK(st[1].code == 2);
    CHECK(st[2].code == 3);
}
