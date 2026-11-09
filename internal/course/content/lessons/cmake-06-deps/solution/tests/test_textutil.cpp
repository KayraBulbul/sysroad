#include <doctest/doctest.h>

#include "textutil/textutil.hpp"

TEST_CASE("count counts lines, words and bytes") {
    CHECK(textutil::count("a b\n") == textutil::Counts{1, 2, 4});
    CHECK(textutil::count("") == textutil::Counts{});
}

TEST_CASE("split_words skips runs of spaces") {
    auto words = textutil::split_words("  to be\tor  ");
    REQUIRE(words.size() == 3);
    CHECK(words[0] == "to");
    CHECK(words[2] == "or");
}

TEST_CASE("trim removes spaces at both ends only") {
    CHECK(textutil::trim("  x y \n") == "x y");
    CHECK(textutil::trim(" \t ").empty());
}
