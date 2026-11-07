#include "doctest.h"

#include <sstream>

#include "config.hpp"

static std::map<std::string, std::string> parse(const std::string& text) {
    std::istringstream in(text);
    return parse_config(in);
}

TEST_CASE("key = value lines") {
    auto cfg = parse("port = 8080\nhost=localhost\n");
    CHECK(cfg.size() == 2);
    CHECK(cfg["port"] == "8080");
    CHECK(cfg["host"] == "localhost");
}

TEST_CASE("a line with no = throws, with its line number") {
    CHECK_THROWS_WITH_AS(parse("a = 1\nnope\n"), "line 2: expected key = value", ConfigError);
}
