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

TEST_CASE("-E marks line ends") {
    CHECK(cat({"-E"}, "a\n\nb\n") == "a$\n$\nb$\n");
}

TEST_CASE("-T shows tabs") {
    CHECK(cat({"-T"}, "a\tb\n") == "a^Ib\n");
}

TEST_CASE("-E shows a Windows line ending")  {
    CHECK(cat({"-E"}, "dos\r\n") == "dos^M$\n");
}

TEST_CASE("-v shows control bytes") {
    CHECK(cat({"-v"}, "x\x01y\n") == "x^Ay\n");
}
