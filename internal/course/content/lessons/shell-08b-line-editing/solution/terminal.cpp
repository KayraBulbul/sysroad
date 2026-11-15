// Reading a line at the terminal, with editing.
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <string>
#include <tuple>

#include "conch.hpp"
#include "keys.hpp"
#include "lineedit.hpp"

namespace {

// Raw mode for as long as it lives: keys arrive one at a time, unechoed,
// and Ctrl-C is just a key.
class RawMode {
  public:
    explicit RawMode(int fd) : fd_(fd) {
        ok_ = tcgetattr(fd, &saved_) == 0;
        termios t = saved_;
        t.c_iflag &= ~static_cast<tcflag_t>(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        t.c_oflag &= ~static_cast<tcflag_t>(OPOST);
        t.c_cflag |= CS8;
        t.c_lflag &= ~static_cast<tcflag_t>(ECHO | ICANON | IEXTEN | ISIG);
        t.c_cc[VMIN] = 1;
        t.c_cc[VTIME] = 0;
        if (ok_) {
            tcsetattr(fd, TCSADRAIN, &t);
        }
    }
    ~RawMode() {
        if (ok_) {
            tcsetattr(fd_, TCSADRAIN, &saved_);
        }
    }
    RawMode(const RawMode&) = delete;
    RawMode& operator=(const RawMode&) = delete;

  private:
    int fd_;
    termios saved_{};
    bool ok_ = false;
};

void write_all(const std::string& s) {
    size_t done = 0;
    while (done < s.size()) {
        ssize_t n = write(1, s.data() + done, s.size() - done);
        if (n == -1 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            return;
        }
        done += static_cast<size_t>(n);
    }
}

// Reads one key, giving the rest of an escape sequence 50 ms to arrive.
// -1 at the end of input.
int read_key(int fd, std::string& pending) {
    char buf[32];
    while (pending.empty()) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n > 0) {
            pending.append(buf, static_cast<size_t>(n));
        } else if (n == 0 || errno != EINTR) {
            return -1;
        }
    }
    auto [key, used] = decode_key(pending);
    pollfd p{fd, POLLIN, 0};
    if (key == kEsc && pending.size() < 8 && poll(&p, 1, 50) > 0) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n > 0) {
            pending.append(buf, static_cast<size_t>(n));
            std::tie(key, used) = decode_key(pending);
        }
    }
    pending.erase(0, used > 0 ? used : 1);
    return key;
}

}  // namespace

std::optional<std::string> read_edited_line(Shell& sh, const std::string& prompt) {
    RawMode raw(sh.tty);
    LineEditor editor(sh.history);
    // Bytes read but not used yet: keys typed ahead, or a paste, belong to
    // the next lines, so they outlive this one.
    static std::string pending;
    write_all(editor.render(prompt));
    for (;;) {
        int key = read_key(sh.tty, pending);
        if (key == -1) {
            write_all("\r\n");
            return std::nullopt;
        }
        LineEditor::Result r = editor.feed(key);
        if (r == LineEditor::Done) {
            write_all("\r\n");
            return editor.text();
        }
        if (r == LineEditor::EndOfInput) {
            write_all("\r\n");
            return std::nullopt;
        }
        if (r == LineEditor::Cancelled) {
            write_all("^C\r\n");
            editor = LineEditor(sh.history);
        }
        write_all(editor.render(prompt));
    }
}
