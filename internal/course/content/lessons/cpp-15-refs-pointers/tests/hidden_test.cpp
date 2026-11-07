#include "doctest.h"

#include <string>
#include <vector>

#include "refs.hpp"

TEST_CASE("find_value: missing, empty, first of several") {
    std::vector<int> v{1, 2, 2, 3};
    CHECK(find_value(v, 7) == nullptr);
    std::vector<int> empty;
    CHECK(find_value(empty, 1) == nullptr);
    CHECK(find_value(v, 2) == &v[1]);
}

TEST_CASE("trim: every kind of whitespace") {
    std::string s = "\t\n hi there \r\n";
    trim(s);
    CHECK(s == "hi there");
}

TEST_CASE("trim: nothing to trim, all whitespace, empty") {
    std::string a = "x", b = " \t\n", c;
    trim(a);
    trim(b);
    trim(c);
    CHECK(a == "x");
    CHECK(b == "");
    CHECK(c == "");
}

TEST_CASE("longest: points into the vector") {
    std::vector<std::string> words{"a", "abc", "ab", "xyz"};
    const std::string* p = longest(words);
    REQUIRE(p != nullptr);
    CHECK(*p == "abc");
    CHECK(p == &words[1]);
}

TEST_CASE("longest: none") {
    CHECK(longest({}) == nullptr);
}

TEST_CASE("swap_values: the same variable twice") {
    int a = 5;
    swap_values(a, a);
    CHECK(a == 5);
}
