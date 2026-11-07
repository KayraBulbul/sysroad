#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "wc.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

static std::string wc(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    run_wc(args, in, out, err);
    return out.str();
}

TEST_CASE("column_width: one file, one column") {
    write_file("a.txt", "hello world\n");
    CHECK(column_width(parse_options({"-l", "a.txt"})) == 1);
}

TEST_CASE("column_width: digits of the total size") {
    write_file("a.txt", "hello world\n");      // 12 bytes
    write_file("b.txt", "one\ntwo three\n");  // 14 bytes
    CHECK(column_width(parse_options({"a.txt", "b.txt"})) == 2);
}

TEST_CASE("run_wc: two files, GNU widths") {
    write_file("a.txt", "hello world\n");
    write_file("b.txt", "one\ntwo three\n");
    CHECK(wc({"a.txt", "b.txt"}) == " 1  2 12 a.txt\n"
                                    " 2  3 14 b.txt\n"
                                    " 3  5 26 total\n");
}
