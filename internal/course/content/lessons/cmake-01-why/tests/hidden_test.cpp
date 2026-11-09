#include "doctest.h"

#include <stdexcept>

#include "build.hpp"

TEST_CASE("editing a header recompiles everything that includes it") {
    Mtimes t{{"main.cpp", 1}, {"greet.cpp", 1}, {"greet.hpp", 9},
             {"main.o", 2},   {"greet.o", 2},   {"app", 3}};
    CHECK(plan(hello_project(), t, "app") == cmds({"make main.o", "make greet.o", "make app"}));
}

TEST_CASE("a deleted object file is rebuilt, and so is what depends on it") {
    Mtimes t{{"main.cpp", 1}, {"greet.cpp", 1}, {"greet.hpp", 1}, {"greet.o", 2}, {"app", 3}};
    CHECK(plan(hello_project(), t, "app") == cmds({"make main.o", "make app"}));
}

TEST_CASE("an older output than its input is stale; the same time is not") {
    Rules r = rules({{"out", {"in"}}});
    CHECK(plan(r, {{"in", 5}, {"out", 4}}, "out") == cmds({"make out"}));
    CHECK(plan(r, {{"in", 5}, {"out", 5}}, "out").empty());
}

TEST_CASE("a target several others need runs once") {
    // gen.hpp is generated, and both objects include it.
    Rules r = rules({
        {"app", {"a.o", "b.o"}},
        {"a.o", {"a.cpp", "gen.hpp"}},
        {"b.o", {"b.cpp", "gen.hpp"}},
        {"gen.hpp", {"schema.txt"}},
    });
    Mtimes t{{"a.cpp", 1}, {"b.cpp", 1}, {"schema.txt", 1}};
    CHECK(plan(r, t, "app") == cmds({"make gen.hpp", "make a.o", "make b.o", "make app"}));
}

TEST_CASE("a rule with no dependencies runs only when its target is missing") {
    Rules r = rules({{"clean", {}}, {"config.h", {}}});
    CHECK(plan(r, {}, "clean") == cmds({"make clean"}));
    CHECK(plan(r, {{"config.h", 1}}, "config.h").empty());
}

TEST_CASE("asking for a source file is fine; asking for a missing one isn't") {
    Rules r = hello_project();
    CHECK(plan(r, {{"main.cpp", 1}}, "main.cpp").empty());
    CHECK_THROWS_WITH_AS(plan(r, {}, "nope"), "no rule to make nope", std::runtime_error);
}

TEST_CASE("cycles are reported with the loop") {
    Rules r = rules({{"a", {"b"}}, {"b", {"c"}}, {"c", {"a"}}});
    CHECK_THROWS_WITH_AS(plan(r, {}, "a"), "cycle: a -> b -> c -> a", std::runtime_error);
    Rules self = rules({{"x", {"y"}}, {"y", {"y"}}});
    CHECK_THROWS_WITH_AS(plan(self, {}, "x"), "cycle: y -> y", std::runtime_error);
}

TEST_CASE("a diamond is not a cycle") {
    Rules r = rules({{"top", {"l", "r"}}, {"l", {"base"}}, {"r", {"base"}}, {"base", {}}});
    CHECK(plan(r, {}, "top") == cmds({"make base", "make l", "make r", "make top"}));
}
