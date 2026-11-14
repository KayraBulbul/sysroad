#include "doctest.h"

#include <filesystem>
#include <set>
#include <unistd.h>

#include "session.hpp"

namespace ktest {

namespace fs = std::filesystem;

static std::set<std::string> files_here() {
    std::set<std::string> names;
    for (const auto& entry : fs::directory_iterator(".")) {
        names.insert(entry.path().filename().string());
    }
    return names;
}

TEST_CASE("Ctrl-Q counts down three more times, and the fourth quits") {
    Kite k(6, 70);
    k.open("q.txt", "q\n");
    k.type("x").press(keys::ctrl_q).press(keys::ctrl_q);
    CHECK(k.line(5) == "Unsaved changes! Press Ctrl-Q 2 more times to quit anyway.");
    k.press(keys::ctrl_q);
    CHECK(k.line(5) == "Unsaved changes! Press Ctrl-Q 1 more time to quit anyway.");
    CHECK_FALSE(k.quit);
    k.press(keys::ctrl_q);
    CHECK(k.quit);
}

TEST_CASE("any other key starts the Ctrl-Q count again") {
    Kite k(6, 70);
    k.open("q.txt", "q\n");
    k.type("x").press(keys::ctrl_q).press(keys::ctrl_q).press(keys::left).press(keys::ctrl_q);
    CHECK(k.line(5) == "Unsaved changes! Press Ctrl-Q 3 more times to quit anyway.");
    CHECK_FALSE(k.quit);
}

TEST_CASE("after a save, Ctrl-Q quits straight away") {
    Kite k(6, 40);
    k.open("q.txt", "q\n");
    k.type("x").press(keys::ctrl_s).press(keys::ctrl_q);
    CHECK(k.quit);
}

TEST_CASE("keys that change nothing don't mark the file modified") {
    Kite k(6, 40);
    k.open("same.txt", "ab\n");
    k.press(keys::backspace).press(keys::end).press(keys::del).press(keys::down).press(keys::backspace);
    CHECK(k.line(4) == "same.txt - 1 lines                   2/1");
}

TEST_CASE("Enter, Backspace and Delete mark it modified too") {
    for (std::string_view key : {keys::enter, keys::del}) {
        Kite k(6, 40);
        k.open("m.txt", "ab\n");
        k.press(key);
        CHECK(contains(k.line(4), "(modified)"));
    }
    Kite k(6, 40);
    k.open("m.txt", "ab\n");
    k.press(keys::end).press(keys::backspace);
    CHECK(contains(k.line(4), "(modified)"));
}

TEST_CASE("saving over a file replaces it, and leaves nothing else behind") {
    write_file("over.txt", "");
    auto before = files_here();
    Kite k(6, 40);
    k.open("over.txt", "an old version that is longer\n");
    k.press(keys::end).press(keys::backspace, 29).type("new\ntext").press(keys::ctrl_s);
    CHECK(read_file("over.txt") == "new\ntext\n");
    CHECK(k.line(5) == "9 bytes written to disk");
    CHECK_MESSAGE(files_here() == before, "a temporary file is still in the folder");
}

TEST_CASE("the save goes through a new file renamed over the old one") {
    write_file("renamed.txt", "old\n");
    fs::remove("renamed-link.txt");
    fs::create_hard_link("renamed.txt", "renamed-link.txt");
    Kite k(6, 40);
    k.open("renamed.txt", "old\n");
    k.press(keys::end).type("er").press(keys::ctrl_s);
    CHECK(read_file("renamed.txt") == "older\n");
    CHECK_MESSAGE(read_file("renamed-link.txt") == "old\n",
                  "a hard link to the old file still sees the old text only if the new text went into a new "
                  "file. Writing over the file in place changed it.");
}

TEST_CASE("an empty file saves as an empty file") {
    Kite k(6, 40);
    open_file(k.e, "empty-new.txt");
    k.press(keys::ctrl_s);
    CHECK(fs::exists("empty-new.txt"));
    CHECK(read_file("empty-new.txt") == "");
    CHECK(k.line(5) == "0 bytes written to disk");
}

TEST_CASE("a save that can't happen says so, and the file stays modified") {
    Kite k(6, 50);
    open_file(k.e, "no-such-folder/inside.txt");
    k.type("x").press(keys::ctrl_s);
    CHECK(k.line(5).rfind("Can't save!", 0) == 0);
    CHECK(contains(k.line(4), "(modified)"));
    CHECK(files_here().count("no-such-folder") == 0);
}

TEST_CASE("a save whose rename fails removes its temporary file, and the file stays modified") {
    fs::create_directory("a-folder.txt");  // a file can't be renamed over a folder
    auto before = files_here();
    Kite k(6, 50);
    open_file(k.e, "a-folder.txt");
    k.type("x").press(keys::ctrl_s);
    CHECK(k.line(5).rfind("Can't save!", 0) == 0);
    CHECK(contains(k.line(4), "(modified)"));
    CHECK_MESSAGE(files_here() == before, "the temporary file is still in the folder");
}

TEST_CASE("with no name, Ctrl-S asks for one in the message bar") {
    Kite k(6, 40);
    k.draw().type("hi").press(keys::ctrl_s);
    CHECK(k.line(5) == "Save as:");
    k.type("out.txt");
    CHECK(k.line(5) == "Save as: out.txt");
    k.press(keys::backspace, 3).type("md");
    CHECK(k.line(5) == "Save as: out.md");
    k.press(keys::enter);
    CHECK(read_file("out.md") == "hi\n");
    CHECK(k.line(4) == "out.md - 1 lines                     1/1");
    CHECK(k.line(5) == "3 bytes written to disk");
}

TEST_CASE("while it asks, keys go to the answer, not the text") {
    Kite k(6, 40);
    k.draw().type("hi").press(keys::ctrl_s);
    k.press(keys::left).press(keys::enter).press(keys::ctrl_q);
    CHECK_FALSE_MESSAGE(k.quit, "Ctrl-Q in the prompt shouldn't quit");
    CHECK(k.line(0) == "hi");
    CHECK_MESSAGE(k.line(5) == "Save as:", "Enter with no answer keeps asking");
}

TEST_CASE("Escape doesn't save") {
    Kite k(6, 40);
    k.draw().type("hi").press(keys::ctrl_s).type("never.txt").press(keys::esc);
    CHECK_FALSE(fs::exists("never.txt"));
    CHECK(k.line(5) == "Not saved");
    CHECK(k.line(4) == "[No Name] - 1 lines (modified)       1/1");
    k.type("!");
    CHECK(k.line(0) == "hi!");
}

}  // namespace ktest
