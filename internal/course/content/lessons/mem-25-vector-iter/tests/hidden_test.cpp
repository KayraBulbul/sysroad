#include "doctest.h"

#include <algorithm>
#include <string>
#include <vector>

#include "counted.hpp"
#include "vector.hpp"

static std::vector<std::string> as_std(const Vector<std::string>& v) {
    return std::vector<std::string>(v.begin(), v.end());
}

TEST_CASE("insert while growing, with a value from the vector itself") {
    Vector<std::string> v;
    v.push_back("a long string that lives on the heap, number one");
    for (int i = 0; i < 6; ++i) {
        v.insert(v.begin(), v.back());  // capacity runs out on some of these
    }
    CHECK(v.size() == 7);
    for (const auto& s : v) {
        CHECK(s == "a long string that lives on the heap, number one");
    }
}

TEST_CASE("insert into an empty vector, and at every position") {
    Vector<int> v;
    v.insert(v.begin(), 1);
    CHECK(v.size() == 1);
    for (int i = 0; i <= 5; ++i) {
        Vector<int> w = {10, 20, 30, 40, 50};
        auto it = w.insert(w.begin() + i, 99);
        CHECK(it == w.begin() + i);
        CHECK(w[i] == 99);
        CHECK(w.size() == 6);
    }
}

TEST_CASE("erase destroys exactly what it removes") {
    Counted::reset();
    {
        Vector<Counted> v;
        for (int i = 0; i < 10; ++i) {
            v.emplace_back(std::to_string(i));
        }
        v.erase(v.begin() + 2);
        CHECK(Counted::live == 9);
        v.erase(v.begin() + 1, v.begin() + 5);
        CHECK(Counted::live == 5);
        CHECK(v[0].value == "0");
        CHECK(v[1].value == "6");
        auto it = v.erase(v.end() - 1);
        CHECK(it == v.end());
        CHECK(v.erase(v.begin(), v.begin()) == v.begin());  // an empty range
        CHECK(Counted::live == 4);
        CHECK(Counted::copies == 0);
    }
    CHECK(Counted::live == 0);
}

TEST_CASE("erase everything") {
    Vector<std::string> v = {"a", "b", "c"};
    auto it = v.erase(v.begin(), v.end());
    CHECK(it == v.end());
    CHECK(v.empty());
}

TEST_CASE("erase_if keeps the order, and handles all or nothing") {
    Vector<std::string> v = {"keep1", "drop", "keep2", "drop", "drop", "keep3"};
    CHECK(erase_if(v, [](const std::string& s) { return s == "drop"; }) == 3);
    CHECK(as_std(v) == std::vector<std::string>{"keep1", "keep2", "keep3"});
    CHECK(erase_if(v, [](const std::string&) { return false; }) == 0);
    CHECK(v.size() == 3);
    CHECK(erase_if(v, [](const std::string&) { return true; }) == 3);
    CHECK(v.empty());
}

TEST_CASE("const iteration") {
    const Vector<int> v = {4, 5, 6};
    CHECK(*std::max_element(v.begin(), v.end()) == 6);
    CHECK(v.end() - v.begin() == 3);
}

TEST_CASE("the erase-in-a-loop pattern") {
    Vector<int> v;
    for (int i = 0; i < 1000; ++i) {
        v.push_back(i % 3);
    }
    for (auto it = v.begin(); it != v.end();) {
        it = *it == 0 ? v.erase(it) : it + 1;
    }
    CHECK(v.size() == 666);
    CHECK(std::count(v.begin(), v.end(), 0) == 0);
}
