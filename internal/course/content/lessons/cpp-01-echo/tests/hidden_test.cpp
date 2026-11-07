#include "doctest.h"

#include <string>
#include <vector>

std::string join_args(const std::vector<std::string>& args);

TEST_CASE("join_args: one argument") {
    CHECK(join_args({"solo"}) == "solo");
}

TEST_CASE("join_args: keeps spaces inside an argument") {
    CHECK(join_args({"a  b", "c"}) == "a  b c");
}

TEST_CASE("join_args: empty arguments still get separators") {
    CHECK(join_args({"", "x", ""}) == " x ");
}
