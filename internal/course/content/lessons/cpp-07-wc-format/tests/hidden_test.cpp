#include "doctest.h"

#include <filesystem>
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

TEST_CASE("column_width: stdin means at least 7") {
    CHECK(column_width(parse_options({})) == 7);
    write_file("s.txt", "x\n");
    CHECK(column_width(parse_options({"s.txt", "-"})) == 7);
}

TEST_CASE("column_width: stdin with one column is 1") {
    CHECK(column_width(parse_options({"-l"})) == 1);
    CHECK(column_width(parse_options({"-c", "-"})) == 1);
}

TEST_CASE("column_width: a directory means at least 7") {
    std::filesystem::create_directory("adir");
    write_file("t.txt", "hi\n");
    CHECK(column_width(parse_options({"adir", "t.txt"})) == 7);
}

TEST_CASE("column_width: missing files count for nothing") {
    write_file("twelve.txt", "hello world\n");
    CHECK(column_width(parse_options({"nope", "twelve.txt"})) == 2);
    CHECK(column_width(parse_options({"nope1", "nope2"})) == 1);
}

TEST_CASE("column_width: an empty file") {
    write_file("empty.txt", "");
    CHECK(column_width(parse_options({"empty.txt"})) == 1);
}

TEST_CASE("column_width: big files") {
    write_file("big.txt", std::string(123456, 'x'));
    write_file("small.txt", "hello world\n");
    CHECK(column_width(parse_options({"big.txt", "small.txt"})) == 6);
}

TEST_CASE("run_wc: stdin keeps 7, or 1 for a single column") {
    CHECK(wc({}, "hello world\n") == "      1       2      12\n");
    CHECK(wc({"-l"}, "hello world\n") == "1\n");
}

TEST_CASE("run_wc: one file") {
    write_file("one.txt", "hello world\n");
    CHECK(wc({"one.txt"}) == " 1  2 12 one.txt\n");
    CHECK(wc({"-l", "one.txt"}) == "1 one.txt\n");
    CHECK(wc({"-lc", "one.txt"}) == " 1 12 one.txt\n");
}

TEST_CASE("run_wc: the total uses the same width") {
    write_file("w1.txt", std::string(99, 'a') + "\n");  // 100 bytes
    write_file("w2.txt", "b\n");
    CHECK(wc({"-c", "w1.txt", "w2.txt"}) == "100 w1.txt\n"
                                            "  2 w2.txt\n"
                                            "102 total\n");
}

TEST_CASE("format_counts: a run-time width") {
    Options all{.lines = true, .words = true, .bytes = true};
    CHECK(format_counts({1, 2, 3}, all, 3) == "  1   2   3");
    CHECK(format_counts({10, 200, 3000}, all, 1) == "10 200 3000");
}
