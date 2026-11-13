// You won't need to change this file.
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>

#include "keys.hpp"
#include "lineedit.hpp"
#include "shell.hpp"

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
        if (n <= 0 && errno != EINTR) {
            return;
        }
        done += n > 0 ? static_cast<size_t>(n) : 0;
    }
}

// Reads one key, waiting for the rest of an escape sequence.
int read_key(int fd, std::string& pending) {
    char buf[32];
    while (pending.empty()) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n == 0) {
            return -1;
        }
        if (n < 0 && errno != EINTR) {
            return -1;
        }
        if (n > 0) {
            pending.append(buf, static_cast<size_t>(n));
        }
    }
    KeyPress k = decode_key(pending);
    // An arrow key is several bytes (ESC [ D), and they can arrive in
    // separate reads. If what's here so far decodes as a lone Escape, give
    // the rest 50 ms to turn up before believing it.
    while (k.key == kEsc && k.used == 1 && pending.size() < 8) {
        pollfd p{fd, POLLIN, 0};
        if (poll(&p, 1, 50) <= 0) {
            break;
        }
        ssize_t n = read(fd, buf, sizeof buf);
        if (n <= 0) {
            break;
        }
        pending.append(buf, static_cast<size_t>(n));
        KeyPress again = decode_key(pending);
        if (again.key == kEsc && again.used == 1 && pending.size() > 1 && pending[1] != '[' &&
            pending[1] != 'O') {
            break;  // Escape followed by an ordinary key
        }
        k = again;
    }
    if (k.used == 0) {
        k.used = 1;
    }
    pending.erase(0, k.used);
    return k.key;
}

}  // namespace

std::optional<std::string> read_line(Shell& sh, const std::string& prompt) {
    RawMode raw(sh.tty);
    LineEditor editor(sh.history);
    std::string pending;
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
