#include "doctest.h"

#include <sys/stat.h>
#include <unistd.h>

#include <climits>
#include <cstdlib>
#include <fstream>
#include <string>

#include "conch_pty.hpp"

static std::string here() {
    char dir[PATH_MAX];
    return getcwd(dir, sizeof dir);
}

TEST_CASE("on a terminal: edit a line, recall it with Up, and keep the history") {
    std::string home = here() + "/home";
    mkdir(home.c_str(), 0755);
    setenv("HOME", home.c_str(), 1);
    {
        std::ofstream(home + "/.conch_history") << "echo from last time\n";
    }
    Conch c;
    REQUIRE(c.prompt());
    c.send("echo wrld");
    c.send("\x1b[D\x1b[D\x1b[D");  // Left three times
    c.send("o\r");
    CHECK_MESSAGE(c.expect("world"), c.output());
    REQUIRE(c.prompt());
    c.send("\x1b[A\x1b[A\r");  // Up twice: the line from the file
    CHECK_MESSAGE(c.expect("from last time"), c.output());
    REQUIRE(c.prompt());
    c.send("exit\r");
    CHECK(c.wait() == 0);
    std::ifstream in(home + "/.conch_history");
    std::string all((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(all == "echo from last time\necho world\necho from last time\nexit\n");
}
