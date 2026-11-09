#include "doctest.h"

#include <vector>

#include "textutil/textutil.hpp"

TEST_CASE("total adds counts up") {
    std::vector<textutil::Counts> parts{{1, 2, 3}, {10, 20, 30}};
    CHECK(textutil::total(parts) == textutil::Counts{11, 22, 33});
    CHECK(textutil::total({}) == textutil::Counts{});
}

TEST_CASE("empty input") {
    CHECK(textutil::count("") == textutil::Counts{});
    CHECK(textutil::split_words(" \n ").empty());
    CHECK(textutil::trim("   ").empty());
}
