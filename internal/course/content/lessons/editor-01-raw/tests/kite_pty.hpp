#pragma once
// kite on a terminal of its own: a pseudo-terminal, the kind your terminal
// emulator gives a shell. The tests start your main() on it, type at it,
// and read what it draws through term.hpp's Term, as a person would see it.

#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#include "term.hpp"

// Your main, renamed for the tests: kite's is int main(int argc, char** argv).
int sysroad_student_main(int argc, char** argv);

namespace ktest {

class KitePty {
  public:
    // Starts kite with args on a rows x cols terminal. What was on the
    // screen before (the shell's lines) is `before`, one row per line.
    KitePty(int rows, int cols, std::vector<std::string> args = {}, std::vector<std::string> before = {})
        : term_(rows, cols) {
        for (const auto& line : before) {
            term_.write(line + "\r\n");
        }
        master_ = ::posix_openpt(O_RDWR | O_NOCTTY);
        if (master_ == -1 || ::grantpt(master_) == -1 || ::unlockpt(master_) == -1) {
            std::perror("posix_openpt");
            std::abort();
        }
        winsize ws{};
        ws.ws_row = static_cast<unsigned short>(rows);
        ws.ws_col = static_cast<unsigned short>(cols);
        ::ioctl(master_, TIOCSWINSZ, &ws);
        std::string slave = ::ptsname(master_);
        // The tests' own view of the terminal, to read its settings.
        tty_ = ::open(slave.c_str(), O_RDWR | O_NOCTTY);
        std::fflush(nullptr);
        pid_ = ::fork();
        if (pid_ == 0) {
            ::close(master_);
            ::close(tty_);
            ::setsid();
            int fd = ::open(slave.c_str(), O_RDWR);
            ::ioctl(fd, TIOCSCTTY, 0);
            ::dup2(fd, 0);
            ::dup2(fd, 1);
            ::dup2(fd, 2);
            if (fd > 2) {
                ::close(fd);
            }
            for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU, SIGCHLD, SIGPIPE}) {
                ::signal(sig, SIG_DFL);
            }
            sigset_t none;
            sigemptyset(&none);
            ::sigprocmask(SIG_SETMASK, &none, nullptr);
            args.insert(args.begin(), "kite");
            std::vector<char*> argv;
            for (auto& a : args) {
                argv.push_back(a.data());
            }
            argv.push_back(nullptr);
            int r = sysroad_student_main(static_cast<int>(args.size()), argv.data());
            std::fflush(nullptr);
            ::_exit(r);
        }
    }

    ~KitePty() {
        if (pid_ > 0 && !reaped_) {
            ::kill(-pid_, SIGKILL);
            ::kill(pid_, SIGKILL);
            ::waitpid(pid_, nullptr, 0);
        }
        ::close(master_);
        ::close(tty_);
    }

    KitePty(const KitePty&) = delete;
    KitePty& operator=(const KitePty&) = delete;

    // Types bytes at the terminal, as the keyboard sends them.
    void type(const std::string& bytes) { ::write(master_, bytes.data(), bytes.size()); }

    // Reads what kite draws until it has been quiet for a moment.
    const Term& settle(int quiet_ms = 150, int max_ms = 3000) {
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(max_ms);
        while (std::chrono::steady_clock::now() < until && read_some(quiet_ms)) {
        }
        return term_;
    }

    // Waits up to ms for f(term) to be true, reading what kite draws.
    template <class F> bool wait_until(F f, int ms = 3000) {
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        while (!f(term_)) {
            if (std::chrono::steady_clock::now() > until) {
                return false;
            }
            read_some(20);
        }
        return true;
    }

    const Term& term() const { return term_; }

    // What kite wrote, for a failure message, in one line: a sanitizer's
    // error if there was one, else the end of what it printed, without the
    // escape codes.
    std::string shown() const {
        std::string text;
        for (std::size_t i = 0; i < transcript_.size(); ++i) {
            char c = transcript_[i];
            if (c == '\x1b') {  // skip an escape sequence
                ++i;
                if (i < transcript_.size() && transcript_[i] == '[') {
                    while (i + 1 < transcript_.size() && !(transcript_[i + 1] >= 0x40 && transcript_[i + 1] <= 0x7e)) {
                        ++i;
                    }
                    ++i;
                }
                continue;
            }
            bool space = static_cast<unsigned char>(c) <= ' ';
            if (!space) {
                text += c;
            } else if (!text.empty() && text.back() != ' ') {
                text += ' ';
            }
        }
        for (std::string_view mark : {"runtime error:", "ERROR: AddressSanitizer", "ERROR: LeakSanitizer"}) {
            std::size_t at = text.find(mark);
            if (at != std::string::npos) {
                std::size_t from = text.rfind(' ', at > 1 ? at - 2 : 0);
                from = from == std::string::npos ? 0 : from + 1;
                std::size_t end = text.find(" #0 ", at);  // the stack trace starts there
                return "kite stopped with: " + text.substr(from, end == std::string::npos ? 220 : end - from);
            }
        }
        if (text.empty()) {
            return "kite printed nothing.";
        }
        return "The last thing kite printed: " + (text.size() > 160 ? "..." + text.substr(text.size() - 160) : text);
    }

    // The terminal's settings right now.
    termios modes() const {
        termios t{};
        ::tcgetattr(tty_, &t);
        return t;
    }

    // Waits up to ms for kite to turn echo off: it's in raw mode.
    bool wait_for_raw_mode(int ms = 3000) {
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        while (modes().c_lflag & ECHO) {
            if (std::chrono::steady_clock::now() > until) {
                return false;
            }
            read_some(10);
        }
        return true;
    }

    // Waits up to ms for kite to exit. Returns its exit code, or -1 if it
    // was killed (by a signal) or didn't exit in time.
    int wait_exit(int ms = 3000) {
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        int st = 0;
        while (::waitpid(pid_, &st, WNOHANG) == 0) {
            if (std::chrono::steady_clock::now() > until) {
                return -1;
            }
            read_some(10);  // keep the terminal drained so kite can't block on it
        }
        reaped_ = true;
        while (read_some(0)) {
        }
        return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
    }

  private:
    bool read_some(int ms) {
        pollfd p{master_, POLLIN, 0};
        if (::poll(&p, 1, ms) <= 0) {
            return false;
        }
        char buf[4096];
        ssize_t n = ::read(master_, buf, sizeof buf);
        if (n <= 0) {
            return false;
        }
        term_.write(std::string_view(buf, static_cast<std::size_t>(n)));
        transcript_.append(buf, static_cast<std::size_t>(n));
        std::string replies = term_.take_replies();
        if (!replies.empty()) {
            type(replies);
        }
        return true;
    }

    Term term_;
    std::string transcript_;  // everything kite wrote
    int master_ = -1;
    int tty_ = -1;
    pid_t pid_ = -1;
    bool reaped_ = false;
};

}  // namespace ktest
