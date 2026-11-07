#include "doctest.h"

#include <filesystem>
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

TEST_CASE("run_grep: two files get names") {
    write_file("a.log", "error: one\nfine\n");
    write_file("b.log", "fine\nerror: two\n");
    Ran r = grep({"error", "a.log", "b.log"});
    CHECK(r.out == "a.log:error: one\nb.log:error: two\n");
    CHECK(r.status == 0);
}

TEST_CASE("run_grep: one file, no name") {
    write_file("one.txt", "x\nxy\n");
    CHECK(grep({"y", "one.txt"}).out == "xy\n");
}

TEST_CASE("run_grep: - is (standard input)") {
    write_file("f.txt", "hello file\n");
    Ran r = grep({"hello", "-", "f.txt"}, "hello stdin\n");
    CHECK(r.out == "(standard input):hello stdin\nf.txt:hello file\n");
}

TEST_CASE("run_grep: a missing file is status 2, even with matches") {
    write_file("ok.txt", "match\n");
    Ran r = grep({"match", "ok.txt", "nope.txt"});
    CHECK(r.out == "ok.txt:match\n");
    CHECK(r.err == "grep: nope.txt: No such file or directory\n");
    CHECK(r.status == 2);
}

TEST_CASE("run_grep: a directory") {
    std::filesystem::create_directory("adir");
    Ran r = grep({"x", "adir"});
    CHECK(r.err == "grep: adir: Is a directory\n");
    CHECK(r.status == 2);
}

TEST_CASE("run_grep: no arguments") {
    Ran r = grep({});
    CHECK(r.err == "Usage: grep [OPTION]... PATTERNS [FILE]...\n");
    CHECK(r.status == 2);
}

TEST_CASE("run_grep: an empty pattern matches every line") {
    CHECK(grep({""}, "a\n\nb\n").out == "a\n\nb\n");
}

TEST_CASE("run_grep: a last line without a newline gets one") {
    CHECK(grep({"end"}, "start\nthe end").out == "the end\n");
}

TEST_CASE("run_grep: matching is case-sensitive") {
    CHECK(grep({"Cat"}, "cat\nCat\nCAT\n").out == "Cat\n");
}
