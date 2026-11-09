// You won't need to change this file.
#include "terminal.hpp"

#include <cerrno>
#include <stdexcept>
#include <sys/ioctl.h>
#include <system_error>
#include <unistd.h>

#include "keys.hpp"

RawMode::RawMode(int fd) : fd_(fd) {
    if (!isatty(fd) || tcgetattr(fd, &saved_) == -1) {
        throw std::runtime_error(
            "kite needs a terminal: run it from a shell, not with input from a pipe or file");
    }
    termios raw = raw_attrs(saved_);
    if (tcsetattr(fd, TCSAFLUSH, &raw) == -1) {
        throw std::system_error(errno, std::generic_category(), "tcsetattr");
    }
}

RawMode::~RawMode() {
    tcsetattr(fd_, TCSAFLUSH, &saved_);
}

// Reads what's available. With VMIN 0 and VTIME 1, read returns after a
// tenth of a second even when nothing came, so wait loops.
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
    KeyPress k = decode_key(pending_);
    // An escape sequence can arrive in pieces. If what's here so far is only
    // Escape, give the rest a tenth of a second to turn up.
    if (k.key == kEsc && pending_.size() < 8 && fill(false)) {
        k = decode_key(pending_);
    }
    if (k.used == 0) {
        k.used = 1;
    }
    last_ = pending_.substr(0, k.used);
    pending_.erase(0, k.used);
    return k.key;
}

AltScreen::AltScreen() {
    write_all("\x1b[?1049h");
}

AltScreen::~AltScreen() {
    try {
        write_all("\x1b[?1049l");
    } catch (...) {
    }
}

WindowSize window_size(int fd) {
    winsize ws{};
    if (ioctl(fd, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return {};
    }
    return {ws.ws_row, ws.ws_col};
}

void write_all(std::string_view s) {
    while (!s.empty()) {
        ssize_t n = write(STDOUT_FILENO, s.data(), s.size());
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            throw std::system_error(errno, std::generic_category(), "write");
        }
        s.remove_prefix(static_cast<std::size_t>(n));
    }
}
