#include "doctest.h"

#include <string>
#include <vector>

std::string join_args(const std::vector<std::string>& args);

TEST_CASE("join_args: two words") {
    CHECK(join_args({"hello", "world"}) == "hello world");
}

TEST_CASE("join_args: no arguments") {
    CHECK(join_args({}) == "");
}
