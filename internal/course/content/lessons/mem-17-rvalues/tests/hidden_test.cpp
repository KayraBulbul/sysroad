#include "doctest.h"

#include <string>
#include <utility>
#include <vector>

#include "moves.hpp"

static std::vector<Tracked> make(std::initializer_list<const char*> values) {
    std::vector<Tracked> v;
    v.reserve(values.size());
    for (const char* s : values) {
        v.emplace_back(s);
    }
    return v;
}

TEST_CASE("concat moves everything") {
    auto a = make({"a1", "a2"});
    auto b = make({"b1", "b2", "b3"});
    a.reserve(5);
    Tracked::reset();
    std::vector<Tracked> ab = concat(std::move(a), std::move(b));
    CHECK(Tracked::copies == 0);
    REQUIRE(ab.size() == 5);
    CHECK(ab[0].value == "a1");
    CHECK(ab[2].value == "b1");
    CHECK(ab[4].value == "b3");
}

TEST_CASE("concat still moves when a has to grow") {
    auto a = make({"a"});
    auto b = make({"b", "c", "d", "e"});
    Tracked::reset();
    std::vector<Tracked> ab = concat(std::move(a), std::move(b));
    CHECK(Tracked::copies == 0);
    CHECK(ab.size() == 5);
}

TEST_CASE("longest copies nothing and returns the element itself") {
    auto v = make({"ab", "abcd", "x", "wxyz"});
    Tracked::reset();
    const Tracked& l = longest(v);
    CHECK(Tracked::copies == 0);
    CHECK(Tracked::moves == 0);
    CHECK(&l == &v[1]);  // the first of the two longest
    auto one = make({"only"});
    CHECK(&longest(one) == &one[0]);
}

TEST_CASE("append with an lvalue copies exactly once, at the call") {
    std::vector<Tracked> v;
    v.reserve(4);
    Tracked keep("kept");
    Tracked::reset();
    append(v, keep);
    CHECK(Tracked::copies == 1);  // into the parameter; the rest moves
    CHECK(keep.value == "kept");
}

TEST_CASE("take_last until empty") {
    auto v = make({"1", "2", "3"});
    Tracked::reset();
    std::string got;
    while (!v.empty()) {
        got += take_last(v).value;
    }
    CHECK(got == "321");
    CHECK(Tracked::copies == 0);
}

TEST_CASE("rotate3 twice more brings it back") {
    Tracked a("a"), b("b"), c("c");
    rotate3(a, b, c);
    rotate3(a, b, c);
    rotate3(a, b, c);
    CHECK(a.value == "a");
    CHECK(b.value == "b");
    CHECK(c.value == "c");
}
