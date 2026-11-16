#pragma once
// conch on a terminal of its own, for the tests to type into.

#include <chrono>
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

    // Runs a line again and again, up to ms in all, until text shows up
    // after it: for waiting on something conch reports between lines, like
    // a background job that ended or stopped.
    bool line_until(const std::string& s, const std::string& text, int ms = 3000) {
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        while (std::chrono::steady_clock::now() < until) {
            line(s);
            if (expect(text, 250)) {
                return true;
            }
        }
        return false;
    }

    // Runs a line that starts a foreground job, and waits until the job
    // has the terminal, ready for Ctrl-Z or Ctrl-C. "" if it got there,
    // else why not, for a failure message.
    std::string start_job(const std::string& s) {
        line(s);
        if (job_in_foreground()) {
            return "";
        }
        return "\"" + s + "\" never got the terminal: a foreground job needs a process group of its own, "
               "made the terminal's foreground group with tcsetpgrp. The terminal showed: " + output();
    }
};

}  // namespace shtest
