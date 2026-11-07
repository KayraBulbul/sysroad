#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "grep.hpp"

namespace fs = std::filesystem;

static void write_file(const std::string& name, const std::string& data) {
    if (fs::path dir = fs::path(name).parent_path(); !dir.empty()) {
        fs::create_directories(dir);
    }
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

TEST_CASE("-r with no files searches ., without ./") {
    write_file("t3/top.txt", "word\n");
    write_file("t3/deep/er/file.txt", "word\n");
    fs::path old = fs::current_path();
    fs::current_path("t3");
    Ran r = grep({"-r", "word"});
    fs::current_path(old);
    CHECK(r.out == "deep/er/file.txt:word\ntop.txt:word\n");
}

TEST_CASE("-r includes hidden files and skips symlinks") {
    write_file("t4/.hidden/h.txt", "find me\n");
    write_file("t4/real.txt", "find me\n");
    fs::create_symlink("real.txt", "t4/link.txt");
    Ran r = grep({"-r", "find", "t4"});
    CHECK(r.out == "t4/.hidden/h.txt:find me\nt4/real.txt:find me\n");
}

TEST_CASE("-r on a single file: no name") {
    write_file("t5.txt", "solo\n");
    CHECK(grep({"-r", "solo", "t5.txt"}).out == "solo\n");
}

TEST_CASE("-r: a directory and a file") {
    write_file("t6/in.txt", "z\n");
    write_file("t6.txt", "z\n");
    CHECK(grep({"-r", "z", "t6", "t6.txt"}).out == "t6/in.txt:z\nt6.txt:z\n");
}

TEST_CASE("binary files: one message, status 0") {
    write_file("t7/data.bin", std::string("abc\0needle\0", 11));
    write_file("t7/text.txt", "needle\n");
    Ran r = grep({"-r", "needle", "t7"});
    CHECK(r.out == "t7/text.txt:needle\n");
    CHECK(r.err == "grep: t7/data.bin: binary file matches\n");
    CHECK(r.status == 0);
}

TEST_CASE("binary files: no match, no message") {
    write_file("t8/data.bin", std::string("abc\0def", 7));
    Ran r = grep({"-r", "zzz", "t8"});
    CHECK(r.err == "");
    CHECK(r.status == 1);
}

TEST_CASE("binary files: zero bytes end lines, and -c counts") {
    write_file("t9.bin", std::string("cat\0bin\n", 8));
    CHECK(grep({"-c", "-v", "dog", "t9.bin"}).out == "2\n");
    Ran r = grep({"-v", "at", "t9.bin"});
    CHECK(r.out == "");
    CHECK(r.err == "grep: t9.bin: binary file matches\n");
}

TEST_CASE("-rc counts every file") {
    write_file("t10/a.txt", "x\nx\n");
    write_file("t10/b.txt", "y\n");
    CHECK(grep({"-rc", "x", "t10"}).out == "t10/a.txt:2\nt10/b.txt:0\n");
}

TEST_CASE("-ri") {
    write_file("t11/f.txt", "Needle\nneedle\nNEEDLE\nhay\n");
    CHECK(grep({"-ri", "needle", "t11"}).out == "t11/f.txt:Needle\nt11/f.txt:needle\nt11/f.txt:NEEDLE\n");
}

TEST_CASE("without -r a directory is still an error") {
    fs::create_directories("t12");
    Ran r = grep({"x", "t12"});
    CHECK(r.err == "grep: t12: Is a directory\n");
    CHECK(r.status == 2);
}

TEST_CASE("-r: an empty directory is no match") {
    fs::create_directories("t13/empty/deeper");
    CHECK(grep({"-r", "x", "t13"}).status == 1);
}
