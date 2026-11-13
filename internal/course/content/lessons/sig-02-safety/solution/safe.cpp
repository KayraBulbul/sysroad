#include "safe.hpp"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>
#include <system_error>

size_t format_line(char* buf, size_t size, const char* text, long n) {
    size_t len = 0;
    auto put = [&](char c) {
        if (len < size) {
            buf[len++] = c;
        }
    };
    for (const char* p = text; *p != '\0'; ++p) {
        put(*p);
    }
    // Digits come out backwards. Work with the negative value, which also
    // covers LONG_MIN, whose positive doesn't fit in a long.
    char digits[24];
    int count = 0;
    long v = n < 0 ? n : -n;
    do {
        digits[count++] = static_cast<char>('0' - v % 10);
        v /= 10;
    } while (v != 0);
    if (n < 0) {
        put('-');
    }
    while (count > 0) {
        put(digits[--count]);
    }
    put('\n');
    return len;
}

// The write end, for the handler.
static volatile int write_fd = -1;

static void to_pipe(int sig) {
    int saved = errno;
    unsigned char byte = static_cast<unsigned char>(sig);
    write(write_fd, &byte, 1);  // if the pipe is full, this signal is dropped
    errno = saved;
}

static void set_flags(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    fcntl(fd, F_SETFD, FD_CLOEXEC);
}

SignalPipe::SignalPipe(std::initializer_list<int> sigs) {
    int p[2];
    if (pipe(p) == -1) {
        throw std::system_error(errno, std::generic_category(), "pipe");
    }
    set_flags(p[0]);
    set_flags(p[1]);
    read_fd_ = p[0];
    write_fd = p[1];
    for (int sig : sigs) {
        struct sigaction sa{};
        sa.sa_handler = to_pipe;
        sa.sa_flags = SA_RESTART;
        sigemptyset(&sa.sa_mask);
        struct sigaction old{};
        sigaction(sig, &sa, &old);
        old_.emplace_back(sig, old);
    }
}

SignalPipe::~SignalPipe() {
    for (const auto& [sig, old] : old_) {
        sigaction(sig, &old, nullptr);
    }
    close(write_fd);
    write_fd = -1;
    close(read_fd_);
}

int SignalPipe::next() {
    unsigned char byte;
    for (;;) {
        ssize_t n = read(read_fd_, &byte, 1);
        if (n == 1) {
            return byte;
        }
        if (n == -1 && errno == EINTR) {
            continue;
        }
        return 0;  // EAGAIN: nothing waiting
    }
}

static const char* name(int sig) {
    switch (sig) {
    case SIGUSR1:
        return "SIGUSR1";
    case SIGINT:
        return "SIGINT";
    case SIGTERM:
        return "SIGTERM";
    default:
        return "?";
    }
}

static volatile sig_atomic_t usr2_count = 0;

// Logs straight from the handler, which only format_line and write allow.
static void log_usr2(int) {
    int saved = errno;
    usr2_count = usr2_count + 1;
    char buf[64];
    size_t n = format_line(buf, sizeof buf, "in the handler: SIGUSR2 #", usr2_count);
    write(1, buf, n);
    errno = saved;
}

// events: echoes the lines on stdin. "raise USR1", "raise USR2", "raise
// INT" and "raise TERM" send that signal to this process. SIGUSR1, SIGINT
// and SIGTERM come back through the SignalPipe; the last two stop it.
int main() {
    SignalPipe signals({SIGUSR1, SIGINT, SIGTERM});
    struct sigaction sa{};
    sa.sa_handler = log_usr2;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR2, &sa, nullptr);

    std::string pending;
    for (;;) {
        pollfd fds[2] = {{0, POLLIN, 0}, {signals.fd(), POLLIN, 0}};
        if (poll(fds, 2, -1) == -1 && errno != EINTR) {
            return 1;
        }
        char buf[4096];
        ssize_t n = fds[0].revents != 0 ? read(0, buf, sizeof buf) : -1;
        if (n > 0) {
            pending.append(buf, n);
        }
        size_t nl;
        bool more = true;
        while (more) {
            for (int sig = signals.next(); sig != 0; sig = signals.next()) {
                std::cout << "signal: " << name(sig) << std::endl;
                if (sig != SIGUSR1) {
                    std::cout << "stopping" << std::endl;
                    return 0;
                }
            }
            more = (nl = pending.find('\n')) != std::string::npos;
            if (!more) {
                break;
            }
            std::string line = pending.substr(0, nl);
            pending.erase(0, nl + 1);
            if (line.rfind("raise ", 0) == 0) {
                std::string what = line.substr(6);
                raise(what == "USR1"   ? SIGUSR1
                      : what == "USR2" ? SIGUSR2
                      : what == "INT"  ? SIGINT
                                       : SIGTERM);
            } else {
                std::cout << "line: " << line << std::endl;
            }
        }
        if (n == 0) {
            std::cout << "end of input" << std::endl;
            return 0;
        }
    }
}
