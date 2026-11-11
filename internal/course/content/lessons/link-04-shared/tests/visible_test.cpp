#include "doctest.h"

#include <cstring>
#include <stdexcept>
#include <string>

#include "library.hpp"
#include "plugins.hpp"

TEST_CASE("load a plugin and call it") {
    Library lib(build_plugin("shout", shout_source));
    CHECK(lib.is_open());
    auto name = lib.function<const char*()>("plugin_name");
    auto apply = lib.function<void(char*)>("plugin_apply");
    CHECK(std::string(name()) == "shout");
    char s[] = "quiet";
    apply(s);
    CHECK(std::string(s) == "QUIET");
}

TEST_CASE("a missing file throws") {
    CHECK_THROWS_AS(Library("/no/such/library.so"), std::runtime_error);
}

TEST_CASE("a missing symbol throws") {
    Library lib(build_plugin("shout", shout_source));
    CHECK_THROWS_AS(lib.symbol("no_such_function"), std::runtime_error);
}
