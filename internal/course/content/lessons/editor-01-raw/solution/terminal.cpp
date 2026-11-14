#include "terminal.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

#include "kite.hpp"

termios raw_attrs(termios t) {
    // Input: no Ctrl-S/Ctrl-Q flow control (IXON), no turning Enter's '\r'
    // into '\n' (ICRNL), and the old serial-line flags nobody needs.
    t.c_iflag &= ~static_cast<tcflag_t>(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // Output: no turning "\n" into "\r\n". kite writes "\r\n" itself.
    t.c_oflag &= ~static_cast<tcflag_t>(OPOST);
    t.c_cflag |= CS8;
    // No echo, no line buffering (ICANON), no Ctrl-C/Ctrl-Z signals (ISIG),
    // no Ctrl-V literal-next (IEXTEN).
    t.c_lflag &= ~static_cast<tcflag_t>(ECHO | ICANON | IEXTEN | ISIG);
    // read returns as soon as there's a byte, or after 1/10 s with none.
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 1;
    return t;
}

RawMode::RawMode(int fd) : fd_(fd) {
    if (!isatty(fd) || tcgetattr(fd, &saved_) == -1) {
        throw std::runtime_error("kite needs a terminal: run it from a shell, not with input from a pipe or file");
    }
    termios raw = raw_attrs(saved_);
    if (tcsetattr(fd, TCSAFLUSH, &raw) == -1) {
        throw std::system_error(errno, std::generic_category(), "tcsetattr");
    }
}

RawMode::~RawMode() {
    tcsetattr(fd_, TCSAFLUSH, &saved_);
}
