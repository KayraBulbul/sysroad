#pragma once
// Runs code on a terminal of its own, a pseudo-terminal, and plays the
// person at the keyboard.

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

class Pty {
  public:
    // Forks a child that leads a new session whose controlling terminal is
    // a fresh pty, with fds 0, 1 and 2 on it. The child runs f() and exits
    // with what it returns.
    template <class F> explicit Pty(F f) {
        master_ = ::posix_openpt(O_RDWR | O_NOCTTY);
        if (master_ == -1 || ::grantpt(master_) == -1 || ::unlockpt(master_) == -1) {
            std::perror("posix_openpt");
            std::abort();
        }
        std::string slave = ::ptsname(master_);
        std::fflush(nullptr);
        pid_ = ::fork();
        if (pid_ == 0) {
            ::close(master_);
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
            int r = f();
            std::fflush(nullptr);
            ::_exit(r);
        }
    }

    ~Pty() {
        if (pid_ > 0 && !reaped_) {
            ::kill(-pid_, SIGKILL);  // the session's first group: the child's own
            ::kill(pid_, SIGKILL);
            ::waitpid(pid_, nullptr, 0);
        }
        ::close(master_);
    }

    Pty(const Pty&) = delete;
    Pty& operator=(const Pty&) = delete;

    // Types s at the terminal.
    void send(const std::string& s) {
        ::write(master_, s.data(), s.size());
    }

    // Reads what the terminal shows until text appears after everything
    // matched so far, or ms milliseconds pass.
    bool expect(const std::string& text, int ms = 2000) {
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        for (;;) {
            size_t at = out_.find(text, seen_);
            if (at != std::string::npos) {
                seen_ = at + text.size();
                return true;
            }
            auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                            until - std::chrono::steady_clock::now())
                            .count();
            if (left <= 0 || !read_some(static_cast<int>(left))) {
                return false;
            }
        }
    }

    // Everything the terminal has shown so far.
    const std::string& output() {
        while (read_some(0)) {
        }
        return out_;
    }

    // Waits up to ms for the child to exit. Returns its exit code, or -1
    // if it was killed or didn't exit in time.
    int wait(int ms = 3000) {
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        int st = 0;
        while (::waitpid(pid_, &st, WNOHANG) == 0) {
            if (std::chrono::steady_clock::now() > until) {
                return -1;
            }
            read_some(10);  // keep the terminal drained so it can't block
        }
        reaped_ = true;
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
            return false;  // EIO on Linux once the terminal has no users
        }
        out_.append(buf, n);
        return true;
    }

    int master_ = -1;
    pid_t pid_ = -1;
    bool reaped_ = false;
    std::string out_;
    size_t seen_ = 0;
};
