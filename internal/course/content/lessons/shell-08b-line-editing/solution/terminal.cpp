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

// Whether bytes that decode as Escape could still become a sequence: a
// lone ESC, or an unfinished ESC [ or ESC O, not too long to be one.
bool could_be_sequence(const std::string& bytes) {
    return bytes.size() == 1 || (bytes.size() < 8 && (bytes[1] == '[' || (bytes[1] == 'O' && bytes.size() == 2)));
}

// Reads one key. While what's here is Escape that could still be the
// start of a sequence, gives the rest 50 ms at a time to arrive: a
// sequence can come in several pieces. -1 at the end of input.
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
    while (key == kEsc && could_be_sequence(pending)) {
        pollfd p{fd, POLLIN, 0};
        if (poll(&p, 1, 50) <= 0) {
            break;  // nothing more came: it was the Escape key
        }
        ssize_t n = read(fd, buf, sizeof buf);
        if (n <= 0) {
            break;
        }
        pending.append(buf, static_cast<size_t>(n));
        std::tie(key, used) = decode_key(pending);
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
