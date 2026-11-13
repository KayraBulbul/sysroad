// conch: a small Unix shell. You won't need to change this file.
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

#include "shell.hpp"

// Reads one line from fd a byte at a time, so the commands it runs can
// read the rest of the input themselves. nullopt at end of input.
static std::optional<std::string> read_plain_line(int fd) {
    std::string line;
    char c;
    for (;;) {
        ssize_t n = read(fd, &c, 1);
        if (n == 1) {
            if (c == '\n') {
                return line;
            }
            line += c;
        } else if (n == 0 || errno != EINTR) {
            if (line.empty()) {
                return std::nullopt;
            }
            return line;
        }
    }
}

int main() {
    Shell sh;
    sh.interactive = isatty(0) && isatty(1);
    if (sh.interactive) {
        sh.tty = 0;
        init_job_control(sh);
    }
    for (;;) {
        notify_jobs(sh);
        if (sh.interactive) {
            std::cout << "conch$ ";
        }
        std::cout.flush();
        std::optional<std::string> line = read_plain_line(0);
        if (!line) {
            break;
        }
        run_line(sh, *line);
        if (sh.exiting) {
            break;
        }
    }
    return sh.exiting ? sh.exit_code : sh.last_status;
}
