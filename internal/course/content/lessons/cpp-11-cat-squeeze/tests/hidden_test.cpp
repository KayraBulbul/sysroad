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

TEST_CASE("-s: empty lines at the start") {
    CHECK(cat({"-s"}, "\n\n\nx\n") == "\nx\n");
}

TEST_CASE("-s: empty lines at the end") {
    CHECK(cat({"-s"}, "x\n\n\n\n") == "x\n\n");
}

TEST_CASE("-s: a run split across two files") {
    write_file("s1.txt", "a\n\n\n");
    write_file("s2.txt", "\n\nb\n");
    CHECK(cat({"-s", "s1.txt", "s2.txt"}) == "a\n\nb\n");
}

TEST_CASE("-s: a line of spaces isn't empty") {
    CHECK(cat({"-s"}, "\n \n\n") == "\n \n\n");
}

TEST_CASE("-sn: squeezed lines don't use up numbers") {
    CHECK(cat({"-sn"}, "a\n\n\n\nb\n") == "     1\ta\n     2\t\n     3\tb\n");
}

TEST_CASE("-sb") {
    CHECK(cat({"-s", "-b"}, "a\n\n\n\nb\n") == "     1\ta\n\n     2\tb\n");
}

TEST_CASE("-s: a long run across a 4096-byte read") {
    std::string in = "start" + std::string(10000, '\n') + "end\n";
    CHECK(cat({"-s"}, in) == "start\n\nend\n");
}
