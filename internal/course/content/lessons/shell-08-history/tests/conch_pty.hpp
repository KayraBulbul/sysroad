#pragma once
// conch on a terminal of its own, for the tests to type into.

#include <cstdlib>
#include <string>

#include "pty.hpp"

int sysroad_student_main();  // conch's main, renamed for the tests

struct Conch : Pty {
    Conch() : Pty([] { return sysroad_student_main(); }) {}

    // Types a line and presses Enter.
    void line(const std::string& s) {
        send(s + "\n");
    }
    bool prompt(int ms = 2000) {
        return expect("conch$ ", ms);
    }
};
