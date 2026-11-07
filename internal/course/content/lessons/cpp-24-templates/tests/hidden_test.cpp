#include "doctest.h"

#include <list>
#include <string>

#include "generic.hpp"

struct Version {
    int major, minor;
    auto operator<=>(const Version&) const = default;  // gives every comparison
};

TEST_CASE("max_of: empty") {
    CHECK(max_of(std::vector<int>{}) == std::nullopt);
}

TEST_CASE("max_of: a struct with comparisons") {
    auto v = max_of(std::vector<Version>{{1, 2}, {2, 0}, {1, 9}});
    REQUIRE(v.has_value());
    CHECK(v->major == 2);
}

TEST_CASE("max_of: negative numbers") {
    CHECK(max_of(std::vector<int>{-5, -2, -9}) == -2);
}

TEST_CASE("Stack: pop on empty") {
    Stack<std::string> s;
    CHECK(s.pop() == std::nullopt);
    CHECK(s.size() == 0);
}

TEST_CASE("Stack: top doesn't remove") {
    Stack<std::string> s;
    s.push("a");
    s.push("b");
    CHECK(s.top() == "b");
    CHECK(s.size() == 2);
}

TEST_CASE("Stack: many elements") {
    Stack<int> s;
    for (int i = 0; i < 1000; ++i) {
        s.push(i);
    }
    for (int i = 999; i >= 0; --i) {
        REQUIRE(s.pop() == i);
    }
    CHECK(s.empty());
}

TEST_CASE("count_where: any iterator") {
    std::vector<int> v{1, 2, 3, 4, 5};
    CHECK(count_where(v.begin(), v.end(), [](int n) { return n > 2; }) == 3);
    std::list<std::string> l{"a", "bb", "ccc"};
    CHECK(count_where(l.begin(), l.end(), [](const std::string& s) { return s.size() >= 2; }) == 2);
    int arr[] = {7, 7, 8};
    CHECK(count_where(arr, arr + 3, [](int n) { return n == 7; }) == 2);
    CHECK(count_where(v.begin(), v.begin(), [](int) { return true; }) == 0);
}
