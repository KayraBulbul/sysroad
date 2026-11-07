#include "doctest.h"

#include "files.hpp"

static std::vector<Entry> sample() {
    return {{"b.txt", 10}, {"a.txt", 30}, {"c.log", 0}, {"d.txt", 10}};
}

TEST_CASE("sort_by_size: biggest first, ties by name") {
    auto v = sample();
    sort_by_size(v);
    CHECK(v[0].name == "a.txt");
    CHECK(v[1].name == "b.txt");
    CHECK(v[2].name == "d.txt");
    CHECK(v[3].name == "c.log");
}

TEST_CASE("total_size and count_at_least") {
    auto v = sample();
    CHECK(total_size(v) == 50);
    CHECK(count_at_least(v, 10) == 3);
}
