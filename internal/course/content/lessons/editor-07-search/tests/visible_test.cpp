#include "doctest.h"

#include "text.hpp"

TEST_CASE("forwards from the start finds the first match") {
    auto rows = lines({"the cat", "a cat sat", "cat"});
    CHECK(at(find(rows, "cat", {0, 0}, 1)) == "0:4");
}

TEST_CASE("a match at `from` counts, and the search goes on past it") {
    auto rows = lines({"the cat", "a cat sat", "cat"});
    CHECK(at(find(rows, "cat", {0, 4}, 1)) == "0:4");
    CHECK(at(find(rows, "cat", {0, 5}, 1)) == "1:2");
}

TEST_CASE("no match at all") {
    auto rows = lines({"the cat"});
    CHECK(at(find(rows, "dog", {0, 0}, 1)) == "none");
}
