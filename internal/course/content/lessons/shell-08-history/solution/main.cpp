// conch: a small Unix shell.
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

#include "conch.hpp"

// Reads one line from fd a byte at a time, so that the commands conch runs
// can read the rest of the input themselves. nullopt at the end of input.
static std::optional<std::string> read_line(int fd) {
    std::string line;
    char c;
    for (;;) {
        ssize_t n = read(fd, &c, 1);
        if (n == 1) {
            if (c == '\n') {
                return line;
            }
            line += c;
        } else if (n == -1 && errno == EINTR) {
            continue;
        } else {
            if (line.empty()) {
                return std::nullopt;
            }
            return line;  // a last line with no newline
        }
    }
}

// Where the history is kept between sessions: ~/.conch_history.
static std::string history_path() {
    const char* home = std::getenv("HOME");
    return home != nullptr ? std::string(home) + "/.conch_history" : "";
}

int main() {
    Shell sh;
    bool interactive = isatty(0);
    if (interactive) {
        sh.interactive = true;
        sh.tty = 0;
        init_job_control(sh);
        if (!history_path().empty()) {
            load_history(sh, history_path());
        }
    }
    for (;;) {
        notify_jobs(sh);  // background jobs that ended since the last line
        if (interactive) {
            std::cout << "conch$ " << std::flush;
        }
        std::optional<std::string> line = read_line(0);
        if (!line) {
            break;
        }
        add_history(sh, *line);
        run_line(sh, *line);
        if (sh.exiting) {
            break;
        }
    }
    if (interactive && !history_path().empty()) {
        save_history(sh, history_path());
    }
    return sh.exiting ? sh.exit_code : sh.last_status;
}
