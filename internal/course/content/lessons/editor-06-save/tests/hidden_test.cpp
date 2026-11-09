#include "doctest.h"

#include <cstdio>
#include <string>

#include "text.hpp"

TEST_CASE("an empty file is empty text") {
    CHECK(rows_to_string({}).empty());
    Editor e = with_lines({""});
    CHECK(rows_to_string(e.rows) == "\n");
}

TEST_CASE("saving over a file replaces it, and leaves no temporary file") {
    const std::string path = "kite_hidden_test.txt";
    {
        std::ofstream out(path);
        out << "an old version that is longer\n";
    }
    Editor e = with_lines({"new", "text"});
    e.filename = path;
    e.dirty = true;
    REQUIRE(save_file(e));
    CHECK(read_file(path) == "new\ntext\n");
    CHECK(e.status_msg == "9 bytes written to disk");
    std::remove(path.c_str());
    // Whatever the temporary file was called, the folder holds nothing else.
    std::ifstream tmp(path + ".kite-save");
    CHECK_FALSE(tmp.good());
}

TEST_CASE("a save that can't happen says so, and the text stays dirty") {
    Editor e = with_lines({"x"});
    e.filename = "no-such-folder/inside.txt";
    e.dirty = true;
    CHECK_FALSE(save_file(e));
    CHECK(e.dirty);
    CHECK(e.status_msg.rfind("Can't save!", 0) == 0);
}

TEST_CASE("the status bar: no name, modified, and the cursor's line") {
    Editor e = with_lines({"a", "b", "c"}, 40);
    e.dirty = true;
    e.cy = 2;
    CHECK(status_bar(e) == "[No Name] - 3 lines (modified)       3/3");
}

TEST_CASE("long names are cut to 20 characters") {
    Editor e = with_lines({"a"}, 50);
    e.filename = "a_very_long_file_name_indeed.txt";
    CHECK(status_bar(e) == "a_very_long_file_nam - 1 lines                 1/1");
}

TEST_CASE("on the line past the end, and with no lines at all") {
    Editor e = with_lines({}, 25);
    CHECK(status_bar(e) == "[No Name] - 0 lines   1/0");
}

TEST_CASE("a narrow window drops the right side, then cuts the left") {
    Editor e = with_lines({"a"}, 22);
    e.filename = "x.txt";
    CHECK(status_bar(e) == "x.txt - 1 lines    1/1");
    e.screen_cols = 18;
    CHECK(status_bar(e) == "x.txt - 1 lines1/1");  // just fits
    e.screen_cols = 17;
    CHECK(status_bar(e) == "x.txt - 1 lines  ");
    e.screen_cols = 10;
    CHECK(status_bar(e) == "x.txt - 1 ");
}
