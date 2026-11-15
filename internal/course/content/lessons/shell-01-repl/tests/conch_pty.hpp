#pragma once
// conch on a terminal of its own, for the tests to type into.

#include <string>

#include "pty.hpp"

// Your main, renamed for the tests: conch's is int main().
int sysroad_student_main();

namespace shtest {

struct Conch : Pty {
    Conch() : Pty([] { return sysroad_student_main(); }) {}

    // Types a line and presses Enter (which sends \r, as a keyboard does).
    void line(const std::string& s) { send(s + "\r"); }

    // Waits for the prompt.
    bool prompt(int ms = 3000) { return expect("conch$ ", ms); }
};

}  // namespace shtest
