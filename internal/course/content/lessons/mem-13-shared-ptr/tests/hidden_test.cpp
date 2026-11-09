#include "doctest.h"

#include <memory>
#include <string>
#include <vector>

#include "cache.hpp"
#include "family.hpp"

TEST_CASE("the cache keeps names apart") {
    std::vector<std::string> asked;
    Cache c([&](const std::string& n) {
        asked.push_back(n);
        return n + "!";
    });
    auto x = c.get("x");
    auto y = c.get("y");
    CHECK(*x == "x!");
    CHECK(*y == "y!");
    CHECK(x != y);
    CHECK(c.loads() == 2);
    auto x2 = c.get("x");
    CHECK(x2 == x);
    CHECK(asked == std::vector<std::string>{"x", "y"});
}

TEST_CASE("the cache doesn't keep things alive") {
    Cache c([](const std::string& n) { return n; });
    std::weak_ptr<const std::string> w;
    {
        auto x = c.get("x");
        w = x;
        CHECK(x.use_count() == 1);  // only the caller owns it
    }
    CHECK(w.expired());
}

TEST_CASE("in_use forgets only the expired") {
    Cache c([](const std::string& n) { return n; });
    auto keep = c.get("keep");
    c.get("drop1");
    c.get("drop2");
    CHECK(c.in_use() == 1);
    CHECK(c.in_use() == 1);
    keep.reset();
    CHECK(c.in_use() == 0);
}

TEST_CASE("adopt: the parent owns, the child only watches") {
    auto p = std::make_shared<Person>(Person{"P", {}, {}});
    auto k = std::make_shared<Person>(Person{"K", {}, {}});
    adopt(p, k);
    REQUIRE(p->children.size() == 1);
    CHECK(p->children[0] == k);
    CHECK(k.use_count() == 2);  // k, and p's children
    CHECK(p.use_count() == 1);  // the child's link doesn't count
}

TEST_CASE("a deep family is freed, and lineage stops at a lost parent") {
    std::vector<std::weak_ptr<Person>> watch;
    auto top = std::make_shared<Person>(Person{"0", {}, {}});
    auto at = top;
    for (int i = 1; i < 6; ++i) {
        auto next = std::make_shared<Person>(Person{std::to_string(i), {}, {}});
        adopt(at, next);
        watch.push_back(next);
        at = next;
    }
    CHECK(lineage(*at) == "5 < 4 < 3 < 2 < 1 < 0");
    auto keep_last = at;
    at.reset();
    top.reset();
    for (std::size_t i = 0; i + 1 < watch.size(); ++i) {
        CHECK(watch[i].expired());
    }
    CHECK(lineage(*keep_last) == "5");  // its parent is gone
}
