#include "terminal.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <tuple>
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

// Reads what's there. With VMIN 0 and VTIME 1, read returns after a tenth
// of a second even when nothing came, so waiting means trying again.
bool KeyReader::fill(bool wait) {
    char buf[64];
    while (true) {
        ssize_t n = read(fd_, buf, sizeof buf);
        if (n > 0) {
            pending_.append(buf, static_cast<std::size_t>(n));
            return true;
        }
        if (n == -1 && errno != EAGAIN && errno != EINTR) {
            throw std::system_error(errno, std::generic_category(), "read");
        }
        if (!wait) {
            return false;
        }
    }
}

int KeyReader::next() {
    if (pending_.empty()) {
        fill(true);
    }
    auto [key, used] = decode_key(pending_);
    // An escape sequence can arrive in pieces. If all that's here so far
    // reads as Escape, give the rest a tenth of a second to turn up.
    if (key == kEsc && pending_.size() < 8 && fill(false)) {
        std::tie(key, used) = decode_key(pending_);
    }
    pending_.erase(0, used);
    return key;
}
