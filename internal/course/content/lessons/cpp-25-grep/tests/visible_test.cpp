#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>

#include "grep.hpp"

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

struct Ran {
    int status;
    std::string out, err;
};

static Ran grep(std::vector<std::string_view> args, const std::string& input = "") {
    std::istringstream in(input);
    std::ostringstream out, err;
    int status = run_grep(args, in, out, err);
    return {status, out.str(), err.str()};
}

TEST_CASE("grep_stream: matching lines, after the prefix") {
    std::istringstream in("one\ntwo\nlone\n");
    std::ostringstream out;
    CHECK(grep_stream(in, "one", "f:", out) == 2);
    CHECK(out.str() == "f:one\nf:lone\n");
}

TEST_CASE("run_grep: stdin") {
    Ran r = grep({"cat"}, "the cat\nthe dog\ncategory\n");
    CHECK(r.out == "the cat\ncategory\n");
    CHECK(r.status == 0);
}

TEST_CASE("run_grep: no match is status 1") {
    Ran r = grep({"zebra"}, "the cat\n");
    CHECK(r.out == "");
    CHECK(r.status == 1);
}
