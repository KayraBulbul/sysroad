#include "doctest.h"

#include "text.hpp"

TEST_CASE("forwards wraps around to the top") {
    auto rows = lines({"cat one", "two", "three cat"});
    CHECK(at(find(rows, "cat", {2, 7}, 1)) == "0:0");
    CHECK(at(find(rows, "cat", {3, 0}, 1)) == "0:0");  // from the line past the end
}

TEST_CASE("several matches on one line, forwards") {
    auto rows = lines({"abababab"});
    CHECK(at(find(rows, "ab", {0, 1}, 1)) == "0:2");
    CHECK(at(find(rows, "ab", {0, 3}, 1)) == "0:4");
    CHECK(at(find(rows, "aba", {0, 5}, 1)) == "0:0");  // overlapping: none starts at 5 or later
}

TEST_CASE("backwards finds the last match at or before `from`") {
    auto rows = lines({"cat cat", "x", "cat cat"});
    CHECK(at(find(rows, "cat", {2, 5}, -1)) == "2:4");
    CHECK(at(find(rows, "cat", {2, 3}, -1)) == "2:0");
    CHECK(at(find(rows, "cat", {1, 0}, -1)) == "0:4");
    CHECK(at(find(rows, "cat", {0, 3}, -1)) == "0:0");
}

TEST_CASE("backwards wraps around to the bottom, and a column of -1 skips the row") {
    auto rows = lines({"cat", "dog", "cat dog cat"});
    CHECK(at(find(rows, "cat", {0, -1}, -1)) == "2:8");
    CHECK(at(find(rows, "dog", {2, 3}, -1)) == "1:0");
}

TEST_CASE("next and previous walk every match in order, the way Ctrl-F's arrows do") {
    auto rows = lines({"a.a", "", "..a"});
    Pos p{0, 0};
    std::string seen;
    for (int i = 0; i < 4; ++i) {
        auto m = find(rows, "a", {p.row, p.col + 1}, 1);
        REQUIRE(m);
        p = *m;
        seen += at(p) + " ";
    }
    CHECK(seen == "0:2 2:2 0:0 0:2 ");
    std::string back;
    for (int i = 0; i < 3; ++i) {
        auto m = find(rows, "a", {p.row, p.col - 1}, -1);
        REQUIRE(m);
        p = *m;
        back += at(p) + " ";
    }
    CHECK(back == "0:0 2:2 0:2 ");
}

TEST_CASE("an empty query, or no text, finds nothing") {
    auto rows = lines({"text"});
    CHECK(at(find(rows, "", {0, 0}, 1)) == "none");
    CHECK(at(find({}, "x", {0, 0}, 1)) == "none");
    CHECK(at(find({}, "x", {0, 0}, -1)) == "none");
}

TEST_CASE("matches don't cross lines, and the case matters") {
    auto rows = lines({"ca", "t", "Cat"});
    CHECK(at(find(rows, "cat", {0, 0}, 1)) == "none");
    CHECK(at(find(rows, "Cat", {0, 0}, -1)) == "2:0");
}

TEST_CASE("a column past the end of the line moves on to the next line") {
    auto rows = lines({"ab", "ab"});
    CHECK(at(find(rows, "ab", {0, 50}, 1)) == "1:0");
    CHECK(at(find(rows, "ab", {1, 50}, -1)) == "1:0");
}
