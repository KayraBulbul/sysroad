#include "doctest.h"

#include <sstream>

#include "config.hpp"

static std::map<std::string, std::string> parse(const std::string& text) {
    std::istringstream in(text);
    return parse_config(in);
}

TEST_CASE("comments, blank lines and spacing") {
    auto cfg = parse("# top\n\n   \n  name =  two words  \n  # indented comment\nx=y\n");
    CHECK(cfg.size() == 2);
    CHECK(cfg["name"] == "two words");
    CHECK(cfg["x"] == "y");
}

TEST_CASE("values can hold = and be empty") {
    auto cfg = parse("url = a=b=c\nempty =\n");
    CHECK(cfg["url"] == "a=b=c");
    CHECK(cfg["empty"] == "");
}

TEST_CASE("an empty key") {
    CHECK_THROWS_WITH_AS(parse("= 5\n"), "line 1: empty key", ConfigError);
}

TEST_CASE("a duplicate key") {
    CHECK_THROWS_WITH_AS(parse("a = 1\n\nb = 2\na = 3\n"), "line 4: duplicate key 'a'",
                         ConfigError);
}

TEST_CASE("ConfigError keeps its line and is a runtime_error") {
    try {
        parse("ok = 1\n# fine\nbad\n");
        FAIL("should have thrown");
    } catch (const std::runtime_error& e) {
        auto* ce = dynamic_cast<const ConfigError*>(&e);
        REQUIRE(ce != nullptr);
        CHECK(ce->line() == 3);
    }
}

TEST_CASE("Windows line endings") {
    auto cfg = parse("a = 1\r\nb = 2\r\n");
    CHECK(cfg["a"] == "1");
    CHECK(cfg["b"] == "2");
}

TEST_CASE("an empty file") {
    CHECK(parse("").empty());
}
