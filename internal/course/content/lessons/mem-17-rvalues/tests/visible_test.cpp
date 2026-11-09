#include "doctest.h"

#include <vector>

#include "moves.hpp"

TEST_CASE("append moves") {
    std::vector<Tracked> v;
    v.reserve(4);
    Tracked::reset();
    append(v, Tracked("x"));
    CHECK(Tracked::copies == 0);
    CHECK(v.back().value == "x");
}

TEST_CASE("take_last moves") {
    std::vector<Tracked> v;
    v.emplace_back("a");
    v.emplace_back("b");
    Tracked::reset();
    Tracked t = take_last(v);
    CHECK(Tracked::copies == 0);
    CHECK(t.value == "b");
    CHECK(v.size() == 1);
}

TEST_CASE("rotate3 moves") {
    Tracked a("1"), b("2"), c("3");
    Tracked::reset();
    rotate3(a, b, c);
    CHECK(Tracked::copies == 0);
    CHECK(a.value == "2");
    CHECK(b.value == "3");
    CHECK(c.value == "1");
}
