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

TEST_CASE("matches: every combination") {
    GrepOptions o;
    o.pattern = "Cat";
    CHECK(matches("a Cat", o));
    CHECK_FALSE(matches("a cat", o));
    o.ignore_case = true;
    CHECK(matches("a CAT", o));
    o.invert = true;
    CHECK_FALSE(matches("a cAt", o));
    CHECK(matches("a dog", o));
}

TEST_CASE("matches: an empty pattern") {
    GrepOptions o;
    o.ignore_case = true;
    CHECK(matches("", o));
    CHECK(matches("x", o));
}

TEST_CASE("-n after the file name") {
    write_file("n1.txt", "x\nhit\n");
    write_file("n2.txt", "hit\n");
    CHECK(grep({"-n", "hit", "n1.txt", "n2.txt"}).out == "n1.txt:2:hit\nn2.txt:1:hit\n");
}

TEST_CASE("-c per file, including zero") {
    write_file("c1.txt", "a\na\n");
    write_file("c2.txt", "b\n");
    Ran r = grep({"-c", "a", "c1.txt", "c2.txt"});
    CHECK(r.out == "c1.txt:2\nc2.txt:0\n");
    CHECK(r.status == 0);
}

TEST_CASE("-c with no match is status 1") {
    Ran r = grep({"-c", "zzz"}, "a\n");
    CHECK(r.out == "0\n");
    CHECK(r.status == 1);
}

TEST_CASE("-vc counts the lines without a match") {
    CHECK(grep({"-vc", "a"}, "a\nb\nc\n").out == "2\n");
}

TEST_CASE("-v: the status follows the selected lines") {
    CHECK(grep({"-v", "a"}, "a\na\n").status == 1);
    CHECK(grep({"-v", "a"}, "a\nb\n").status == 0);
}

TEST_CASE("-in together, flags after the pattern") {
    write_file("t.cpp", "// TODO one\nx\n// todo two\n");
    CHECK(grep({"todo", "t.cpp", "-in"}).out == "1:// TODO one\n3:// todo two\n");
}

TEST_CASE("-i: bytes above 127 aren't letters") {
    CHECK(grep({"-i", "\xc3\xa9"}, "caf\xc3\xa9\nCAF\xc3\x89\n").out == "caf\xc3\xa9\n");
}

TEST_CASE("an unknown flag is status 2") {
    Ran r = grep({"-k", "x"}, "x\n");
    CHECK(r.err == "grep: invalid option -- 'k'\n");
    CHECK(r.status == 2);
}
