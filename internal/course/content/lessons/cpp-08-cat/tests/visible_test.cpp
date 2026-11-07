#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "cat.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

struct Ran {
    int status;
    std::string out, err;
};

static Ran cat(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    int status = run_cat(args, in, out, err);
    return {status, out.str(), err.str()};
}

TEST_CASE("cat_lines: copies lines") {
    std::istringstream in("one\ntwo\n");
    std::ostringstream out;
    cat_lines(in, out);
    CHECK(out.str() == "one\ntwo\n");
}

TEST_CASE("run_cat: stdin with no arguments") {
    Ran r = cat({}, "hello\n");
    CHECK(r.out == "hello\n");
    CHECK(r.status == 0);
}

TEST_CASE("run_cat: two files in order") {
    write_file("a.txt", "first file\n");
    write_file("b.txt", "second file\nline two\n");
    Ran r = cat({"a.txt", "b.txt"});
    CHECK(r.out == "first file\nsecond file\nline two\n");
}
