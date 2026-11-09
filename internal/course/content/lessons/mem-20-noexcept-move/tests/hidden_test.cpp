#include "doctest.h"

#include <string>
#include <utility>
#include <vector>

#include "alloc_count.hpp"
#include "records.hpp"

TEST_CASE("copies still work") {
    Record r(std::string(30, 'r'), {9});
    Record c = r;
    c.scores.push_back(10);
    CHECK(r.scores.size() == 1);
    Line l(std::string(30, 'l'));
    Line m = l;
    CHECK(m.text == l.text);
    m = l;
    CHECK(m.text == l.text);
}

TEST_CASE("move assignment doesn't allocate either") {
    Record a(std::string(50, 'a'), {1, 2});
    Record b(std::string(50, 'b'), {3});
    Line x(std::string(50, 'x'));
    Line y(std::string(50, 'y'));
    AllocScope s;
    b = std::move(a);
    y = std::move(x);
    CHECK(s.news() == 0);
    CHECK(b.name == std::string(50, 'a'));
    CHECK(y.text == std::string(50, 'x'));
}

TEST_CASE("a vector of Lines grows by moving") {
    std::vector<Line> made;
    made.reserve(500);
    for (int i = 0; i < 500; ++i) {
        made.emplace_back(std::string(40, 'q'));
    }
    AllocScope s;
    std::vector<Line> v;
    for (Line& l : made) {
        v.push_back(std::move(l));
    }
    CHECK(s.news() < 20);
}

TEST_CASE("erase in the middle moves the rest down") {
    std::vector<Record> v;
    for (int i = 0; i < 50; ++i) {
        v.emplace_back(std::string(40, 'a' + i % 26), std::vector<int>{i});
    }
    AllocScope s;
    v.erase(v.begin());
    CHECK(s.news() == 0);
    CHECK(v[0].scores[0] == 1);
}
