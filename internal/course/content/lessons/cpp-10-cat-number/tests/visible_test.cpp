#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "cat.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

static std::string cat(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    run_cat(args, in, out, err);
    return out.str();
}

TEST_CASE("-n numbers every line") {
    CHECK(cat({"-n"}, "one\ntwo\n") == "     1\tone\n     2\ttwo\n");
}

TEST_CASE("-b numbers only lines that aren't empty") {
    CHECK(cat({"-b"}, "a\n\nb\n") == "     1\ta\n\n     2\tb\n");
}
