#include "doctest.h"

#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>

#include "conch_run.hpp"
#include "editing.hpp"

namespace shtest {

TEST_CASE("Home, End, Ctrl-A, Ctrl-E and Delete") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("cho [x]" + keys::home + "e" + keys::end + keys::enter);
    CHECK_MESSAGE(printed(c, "[x]"), c.output());
    REQUIRE(c.prompt());
    c.send("eecho [y]" + keys::ctrl_a + keys::del + keys::ctrl_e + keys::enter);
    CHECK_MESSAGE(printed(c, "[y]"), c.output());
}

TEST_CASE("Ctrl-U, Ctrl-K and Ctrl-W") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("garbage" + keys::ctrl_u + "echo [u]" + keys::enter);
    CHECK_MESSAGE(printed(c, "[u]"), c.output());
    REQUIRE(c.prompt());
    c.send("echo [k]xyz" + times(keys::left, 3) + keys::ctrl_k + keys::enter);
    CHECK_MESSAGE(printed(c, "[k]"), c.output());
    REQUIRE(c.prompt());
    c.send("echo [w] two  words  " + keys::ctrl_w + keys::ctrl_w + keys::enter);
    CHECK_MESSAGE(printed(c, "[w]"), c.output());
}

TEST_CASE("Ctrl-C throws the line away and starts a new one") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("echo [no]" + keys::ctrl_c);
    REQUIRE(c.prompt());
    c.send("echo [yes]" + keys::enter);
    CHECK_MESSAGE(printed(c, "[yes]"), c.output());
    CHECK(!contains(c.output(), "\n[no]\r"));
}

TEST_CASE("lines typed ahead, or pasted, all run") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("echo [a]\recho [b]\recho [c]\r");  // in one go, as a paste arrives
    CHECK_MESSAGE(printed(c, "[a]"), c.output());
    CHECK_MESSAGE(printed(c, "[b]"), c.output());
    CHECK_MESSAGE(printed(c, "[c]"), c.output());
}

TEST_CASE("an arrow that arrives in two pieces is still an arrow") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("echo [ab]");
    c.send("\x1b");
    usleep(10'000);
    c.send("[D");  // the rest of Left
    c.send("X" + keys::enter);
    CHECK_MESSAGE(printed(c, "[abX]"), c.output());
}

TEST_CASE("keys that aren't text do nothing to the line") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("echo [k]");
    c.send("\x1b[15~");  // F5
    c.send("\x1b[5~");   // Page Up
    c.send("\x07");      // Ctrl-G
    c.send(keys::enter);
    CHECK_MESSAGE(printed(c, "[k]"), c.output());
}

TEST_CASE("Up stops at the oldest line, and walks a history loaded from the file") {
    std::string home = test_dir() + "/home-edit";
    mkdir(home.c_str(), 0755);
    setenv("HOME", home.c_str(), 1);
    write_file(home + "/.conch_history", "echo [old]\necho [newer]\n");
    Conch c;
    REQUIRE(c.prompt());
    c.send(times(keys::up, 5) + keys::enter);
    CHECK_MESSAGE(printed(c, "[old]"), c.output());
}

TEST_CASE("Backspace at the start and Delete at the end change nothing") {
    Conch c;
    REQUIRE(c.prompt());
    c.send(keys::backspace + keys::del + "echo [z]" + keys::del + keys::right + keys::enter);
    CHECK_MESSAGE(printed(c, "[z]"), c.output());
}

}  // namespace shtest
