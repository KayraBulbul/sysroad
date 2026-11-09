#include "doctest.h"

#include <memory>
#include <string>

#include "cache.hpp"
#include "family.hpp"

TEST_CASE("the cache shares while in use") {
    Cache c([](const std::string& n) { return "<" + n + ">"; });
    auto a = c.get("x");
    auto b = c.get("x");
    REQUIRE(a != nullptr);
    CHECK(*a == "<x>");
    CHECK(a == b);
    CHECK(c.loads() == 1);
    CHECK(c.in_use() == 1);
}

TEST_CASE("the cache lets go") {
    Cache c([](const std::string& n) { return n; });
    c.get("x");  // nobody keeps it
    CHECK(c.in_use() == 0);
    c.get("x");
    CHECK(c.loads() == 2);
}

TEST_CASE("lineage") {
    auto a = std::make_shared<Person>(Person{"A", {}, {}});
    auto b = std::make_shared<Person>(Person{"B", {}, {}});
    auto c = std::make_shared<Person>(Person{"C", {}, {}});
    adopt(a, b);
    adopt(b, c);
    CHECK(lineage(*c) == "C < B < A");
    CHECK(lineage(*a) == "A");
}

TEST_CASE("a family is freed when nobody holds it") {
    std::weak_ptr<Person> watch;
    {
        auto parent = std::make_shared<Person>(Person{"P", {}, {}});
        auto kid = std::make_shared<Person>(Person{"K", {}, {}});
        adopt(parent, kid);
        watch = parent;
    }
    CHECK(watch.expired());
}
