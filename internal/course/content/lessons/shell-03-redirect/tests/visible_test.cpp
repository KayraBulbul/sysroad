#include "doctest.h"

#include "shell.hpp"

using Words = std::vector<std::string>;

TEST_CASE("parse_command: words and redirections") {
    Command c = parse_command(tokenize("sort -r < in.txt 2>> log.txt > out.txt"));
    CHECK(c.argv == Words{"sort", "-r"});
    REQUIRE(c.redirects.size() == 3);
    CHECK(c.redirects[0] == Redirect{Redirect::Read, 0, "in.txt", -1});
    CHECK(c.redirects[1] == Redirect{Redirect::Append, 2, "log.txt", -1});
    CHECK(c.redirects[2] == Redirect{Redirect::Write, 1, "out.txt", -1});
}

TEST_CASE("parse_command: 2>&1") {
    Command c = parse_command(tokenize("make 2>&1"));
    REQUIRE(c.redirects.size() == 1);
    CHECK(c.redirects[0] == Redirect{Redirect::Dup, 2, "", 1});
}
