#include "doctest.h"

#include <stdexcept>

#include "build.hpp"

TEST_CASE("a fresh checkout builds everything, dependencies first") {
    Mtimes t{{"main.cpp", 1}, {"greet.cpp", 1}, {"greet.hpp", 1}};
    CHECK(plan(hello_project(), t, "app") == cmds({"make main.o", "make greet.o", "make app"}));
}

TEST_CASE("nothing changed: nothing to do") {
    Mtimes t{{"main.cpp", 1}, {"greet.cpp", 1}, {"greet.hpp", 1},
             {"main.o", 2},   {"greet.o", 2},   {"app", 3}};
    CHECK(plan(hello_project(), t, "app").empty());
}

TEST_CASE("editing one .cpp recompiles just that file, then links") {
    Mtimes t{{"main.cpp", 1}, {"greet.cpp", 5}, {"greet.hpp", 1},
             {"main.o", 2},   {"greet.o", 2},   {"app", 3}};
    CHECK(plan(hello_project(), t, "app") == cmds({"make greet.o", "make app"}));
}

TEST_CASE("a source file nobody can make is an error") {
    Mtimes t{{"main.cpp", 1}, {"greet.hpp", 1}};
    CHECK_THROWS_WITH_AS(plan(hello_project(), t, "app"), "no rule to make greet.cpp",
                         std::runtime_error);
}
