#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "wc.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

struct Ran {
    int status;
    std::string out, err;
};

static Ran wc(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    int status = run_wc(args, in, out, err);
    return {status, out.str(), err.str()};
}

TEST_CASE("run_wc: stdin when there are no files") {
    Ran r = wc({}, "hello world\n");
    CHECK(r.out == "      1       2      12\n");
    CHECK(r.status == 0);
}

TEST_CASE("run_wc: one file, named") {
    write_file("a.txt", "hello world\n");
    Ran r = wc({"a.txt"});
    CHECK(r.out == "      1       2      12 a.txt\n");
    CHECK(r.err == "");
    CHECK(r.status == 0);
}

TEST_CASE("run_wc: a missing file") {
    Ran r = wc({"nope.txt"});
    CHECK(r.out == "");
    CHECK(r.err == "wc: nope.txt: No such file or directory\n");
    CHECK(r.status == 1);
}
