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

TEST_CASE("-s squeezes empty lines into one") {
    CHECK(cat({"-s"}, "a\n\n\n\nb\n") == "a\n\nb\n");
}

TEST_CASE("-s leaves single empty lines alone") {
    CHECK(cat({"-s"}, "a\n\nb\n\nc\n") == "a\n\nb\n\nc\n");
}
