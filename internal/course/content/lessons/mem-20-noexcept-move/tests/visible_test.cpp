#include "doctest.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "alloc_count.hpp"
#include "records.hpp"

TEST_CASE("both are noexcept movable") {
    CHECK(std::is_nothrow_move_constructible_v<Record>);
    CHECK(std::is_nothrow_move_assignable_v<Record>);
    CHECK(std::is_nothrow_move_constructible_v<Line>);
    CHECK(std::is_nothrow_move_assignable_v<Line>);
}

TEST_CASE("growing a vector of Records moves them") {
    std::vector<Record> made;
    made.reserve(100);
    for (int i = 0; i < 100; ++i) {
        made.emplace_back(std::string(40, 'a' + i % 26), std::vector<int>{1, 2, 3});
    }
    AllocScope s;
    std::vector<Record> v;
    for (Record& r : made) {
        v.push_back(std::move(r));
    }
    CHECK(s.news() < 20);  // only the vector's own buffers
    CHECK(v[99].scores.size() == 3);
}

TEST_CASE("moving a Line doesn't allocate") {
    Line a(std::string(60, 'x'));
    AllocScope s;
    Line b = std::move(a);
    CHECK(s.news() == 0);
    CHECK(b.text.size() == 60);
}
