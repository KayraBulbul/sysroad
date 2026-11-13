#include "safe.hpp"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>
#include <system_error>

size_t format_line(char* buf, size_t size, const char* text, long n) {
    // TODO: copy text, then the digits of n (mind negatives), then '\n'.
    (void)buf;
    (void)size;
    (void)text;
    (void)n;
    return 0;
}

// The write end, for the handler.
static volatile int write_fd = -1;

static void to_pipe(int sig) {
    (void)sig;  // TODO: save errno, write one byte, restore errno
}

SignalPipe::SignalPipe(std::initializer_list<int> sigs) {
    // TODO: pipe, non-blocking and close-on-exec ends, install to_pipe.
    (void)sigs;
    (void)to_pipe;
}

SignalPipe::~SignalPipe() {
    // TODO
}

int SignalPipe::next() {
    return 0;  // TODO: read one byte; 0 if there's nothing to read
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
