#include "doctest.h"

#include <cstdio>

#include "text.hpp"

TEST_CASE("rows_to_string ends every line with a newline") {
    Editor e = with_lines({"one", "", "three"});
    CHECK(rows_to_string(e.rows) == "one\n\nthree\n");
}

TEST_CASE("save_file writes the file and clears dirty") {
    Editor e = with_lines({"saved"});
    e.filename = "kite_visible_test.txt";
    e.dirty = true;
    CHECK(save_file(e));
    CHECK(read_file(e.filename) == "saved\n");
    CHECK_FALSE(e.dirty);
    CHECK(e.status_msg == "6 bytes written to disk");
    std::remove(e.filename.c_str());
}

TEST_CASE("the status bar fills the width") {
    Editor e = with_lines({"a", "b"}, 30);
    e.filename = "notes.txt";
    CHECK(status_bar(e) == "notes.txt - 2 lines        1/2");
}
