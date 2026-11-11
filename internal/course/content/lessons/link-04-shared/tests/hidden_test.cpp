#include "doctest.h"

#include <dlfcn.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "library.hpp"
#include "plugins.hpp"

TEST_CASE("the error says what went wrong") {
    try {
        Library lib("/no/such/library.so");
        FAIL("no exception");
    } catch (const std::runtime_error& e) {
        CHECK(std::string(e.what()).find("/no/such/library.so") != std::string::npos);
    }
}

TEST_CASE("two plugins with the same symbol names stay apart") {
    Library a(build_plugin("shout", shout_source));
    Library b(build_plugin("reverse", reverse_source));
    CHECK(std::string(a.function<const char*()>("plugin_name")()) == "shout");
    CHECK(std::string(b.function<const char*()>("plugin_name")()) == "reverse");
    char s[] = "abc";
    b.function<void(char*)>("plugin_apply")(s);
    CHECK(std::string(s) == "cba");
}

TEST_CASE("a variable is a symbol too") {
    Library lib(build_plugin("reverse", reverse_source));
    int* version = static_cast<int*>(lib.symbol("plugin_version"));
    CHECK(*version == 3);
}

TEST_CASE("moving hands over the handle") {
    Library a(build_plugin("shout", shout_source));
    Library b(std::move(a));
    CHECK_FALSE(a.is_open());
    CHECK(b.is_open());
    CHECK_THROWS_AS(a.symbol("plugin_name"), std::runtime_error);
    CHECK(std::string(b.function<const char*()>("plugin_name")()) == "shout");

    Library c(build_plugin("reverse", reverse_source));
    c = std::move(b);  // closes reverse, takes shout
    CHECK_FALSE(b.is_open());
    CHECK(std::string(c.function<const char*()>("plugin_name")()) == "shout");
    c = std::move(c);
    CHECK(c.is_open());
}

TEST_CASE("a vector of libraries") {
    std::vector<Library> libs;
    libs.emplace_back(build_plugin("shout", shout_source));
    libs.emplace_back(build_plugin("reverse", reverse_source));
    libs.emplace_back(build_plugin("shout", shout_source));  // grows, moving the others
    CHECK(std::string(libs[1].function<const char*()>("plugin_name")()) == "reverse");
}

#if defined(__linux__)
TEST_CASE("the destructor closes the library") {
    std::string path = build_plugin("closing", reverse_source);
    {
        Library lib(path);
        void* again = dlopen(path.c_str(), RTLD_NOW | RTLD_NOLOAD);
        CHECK(again != nullptr);  // loaded
        if (again != nullptr) {
            dlclose(again);
        }
    }
    CHECK(dlopen(path.c_str(), RTLD_NOW | RTLD_NOLOAD) == nullptr);  // and unloaded
}
#endif
