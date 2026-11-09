#include "doctest.h"

#include <cstring>
#include <utility>

#include "alloc_count.hpp"
#include "string.hpp"

TEST_CASE("short strings stay inline") {
    AllocScope s;
    String a("hello");
    String b("exactly fifteen");
    CHECK(a == "hello");
    CHECK(std::strcmp(a.c_str(), "hello") == 0);
    CHECK(b.size() == 15);
    CHECK(b.is_inline());
    CHECK(s.news() == 0);
}

TEST_CASE("long strings go to the heap") {
    String a("sixteen chars!!!");
    CHECK(a.size() == 16);
    CHECK_FALSE(a.is_inline());
    CHECK(a == "sixteen chars!!!");
}

TEST_CASE("push_back grows geometrically") {
    AllocScope s;
    String a;
    for (int i = 0; i < 1000; ++i) {
        a.push_back(char('a' + i % 26));
    }
    CHECK(a.size() == 1000);
    CHECK(a[26] == 'a');
    CHECK(s.news() < 12);
}

TEST_CASE("copies are deep, moves steal") {
    String a("a string long enough for the heap");
    String b = a;
    b[0] = 'A';
    CHECK(a[0] == 'a');
    AllocScope s;
    String c = std::move(b);
    CHECK(s.news() == 0);
    CHECK(c == "A string long enough for the heap");
    CHECK(b.empty());
}
