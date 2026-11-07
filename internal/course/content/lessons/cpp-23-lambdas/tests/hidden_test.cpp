#include "doctest.h"

#include "files.hpp"

static std::vector<Entry> sample() {
    return {{"b.txt", 10}, {"a.txt", 30}, {"c.log", 0}, {"d.txt", 10}};
}

TEST_CASE("largest") {
    auto big = largest(sample());
    REQUIRE(big.has_value());
    CHECK(big->name == "a.txt");
    CHECK_FALSE(largest({}).has_value());
}

TEST_CASE("largest: the first on a tie") {
    auto big = largest({{"x", 5}, {"y", 9}, {"z", 9}});
    REQUIRE(big.has_value());
    CHECK(big->name == "y");
}

TEST_CASE("remove_empty") {
    auto v = sample();
    remove_empty(v);
    CHECK(v.size() == 3);
    for (const Entry& e : v) {
        CHECK(e.size > 0);
    }
    std::vector<Entry> none{{"a", 0}, {"b", 0}};
    remove_empty(none);
    CHECK(none.empty());
}

TEST_CASE("names_with_ext") {
    auto names = names_with_ext(sample(), ".txt");
    CHECK(names == std::vector<std::string>{"b.txt", "a.txt", "d.txt"});
    CHECK(names_with_ext(sample(), ".pdf").empty());
}

TEST_CASE("count_at_least: the edges") {
    auto v = sample();
    CHECK(count_at_least(v, 0) == 4);
    CHECK(count_at_least(v, 31) == 0);
    CHECK(count_at_least({}, 1) == 0);
}

TEST_CASE("total_size: past 4 GiB") {
    std::vector<Entry> v{{"a", 3'000'000'000}, {"b", 3'000'000'000}};
    CHECK(total_size(v) == 6'000'000'000ull);
}

TEST_CASE("sort_by_size: lots of ties stay in name order") {
    std::vector<Entry> v;
    for (char c = 'z'; c >= 'a'; --c) {
        v.push_back({std::string(1, c), 7});
    }
    sort_by_size(v);
    for (std::size_t i = 1; i < v.size(); ++i) {
        CHECK(v[i - 1].name < v[i].name);
    }
}
