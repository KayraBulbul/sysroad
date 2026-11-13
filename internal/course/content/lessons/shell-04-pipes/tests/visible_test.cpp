#include "doctest.h"

#include "shell.hpp"

using Words = std::vector<std::string>;

TEST_CASE("parse: a pipeline") {
    List list = parse(tokenize("ls -l | grep x | wc -l"));
    REQUIRE(list.size() == 1);
    const auto& cmds = list[0].pipeline.commands;
    REQUIRE(cmds.size() == 3);
    CHECK(cmds[0].argv == Words{"ls", "-l"});
    CHECK(cmds[1].argv == Words{"grep", "x"});
    CHECK(cmds[2].argv == Words{"wc", "-l"});
    CHECK(list[0].next == Step::End);
}

TEST_CASE("parse: a list") {
    List list = parse(tokenize("make && ./run || echo failed; echo done"));
    REQUIRE(list.size() == 4);
    CHECK(list[0].next == Step::And);
    CHECK(list[1].next == Step::Or);
    CHECK(list[2].next == Step::Then);
    CHECK(list[3].next == Step::End);
    CHECK(list[3].pipeline.commands[0].argv == Words{"echo", "done"});
}
