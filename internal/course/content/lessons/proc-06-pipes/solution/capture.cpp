#include "capture.hpp"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

static void check(int result, const char* what) {
    if (result == -1) {
        throw std::system_error(errno, std::generic_category(), what);
    }
}

// Starts argv with stdin from in_fd and stdout to out_fd (-1: leave it as
// it is). The child closes the fds in close_in_child: the parent's ends.
static pid_t start_with(const std::vector<std::string>& argv, int in_fd, int out_fd,
                        std::vector<int> close_in_child) {
    std::vector<char*> args;
    for (const auto& a : argv) {
        args.push_back(const_cast<char*>(a.c_str()));
    }
    args.push_back(nullptr);
    std::fflush(nullptr);
    pid_t pid = fork();
    check(pid, "fork");
    if (pid == 0) {
        if (in_fd != -1) {
            dup2(in_fd, 0);
            close(in_fd);
        }
        if (out_fd != -1) {
            dup2(out_fd, 1);
            close(out_fd);
        }
        for (int fd : close_in_child) {
            close(fd);
        }
        signal(SIGPIPE, SIG_DFL);  // an ignored signal stays ignored across exec
        execvp(args[0], args.data());
        std::fprintf(stderr, "%s: %s\n", args[0], std::strerror(errno));
        _exit(127);
    }
    return pid;
}

static bool read_some(int fd, std::string& out) {
    char buf[65536];
    for (;;) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n > 0) {
            out.append(buf, n);
            return true;
        }
        if (n == -1 && errno == EINTR) {
            continue;
        }
        return false;  // end of file (or an error)
    }
}

Output capture(const std::vector<std::string>& argv) {
    int p[2];
    check(pipe(p), "pipe");
    pid_t pid = start_with(argv, -1, p[1], {p[0]});
    close(p[1]);  // or read never sees the end: this would still be a writer
    Output result;
    while (read_some(p[0], result.out)) {
    }
    close(p[0]);
    result.status = wait_for(pid);
    return result;
}

Output communicate(const std::vector<std::string>& argv, const std::string& input) {
    int in[2], out[2];
    check(pipe(in), "pipe");
    if (pipe(out) == -1) {
        int err = errno;
        close(in[0]);
        close(in[1]);
        throw std::system_error(err, std::generic_category(), "pipe");
    }
    pid_t pid = start_with(argv, in[0], out[1], {in[1], out[0]});
    close(in[0]);
    close(out[1]);

    // Writing to a pipe nobody reads raises SIGPIPE, which would kill us.
    // Ignore it while we write, and get EPIPE instead.
    struct sigaction ignore{};
    ignore.sa_handler = SIG_IGN;
    sigemptyset(&ignore.sa_mask);
    struct sigaction old{};
    sigaction(SIGPIPE, &ignore, &old);

    fcntl(in[1], F_SETFL, fcntl(in[1], F_GETFL) | O_NONBLOCK);
    int to_child = in[1];
    if (input.empty()) {
        close(to_child);
        to_child = -1;
    }
    size_t sent = 0;
    Output result;
    bool reading = true;
    while (reading || to_child != -1) {
        pollfd fds[2];
        int n = 0;
        if (reading) {
            fds[n++] = {out[0], POLLIN, 0};
        }
        if (to_child != -1) {
            fds[n++] = {to_child, POLLOUT, 0};
        }
        if (poll(fds, n, -1) == -1) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        for (int i = 0; i < n; ++i) {
            if (fds[i].revents == 0) {
                continue;
            }
            if (fds[i].fd == out[0]) {
                reading = read_some(out[0], result.out);
                continue;
            }
            ssize_t w = write(to_child, input.data() + sent, input.size() - sent);
            if (w > 0) {
                sent += w;
            }
            bool retry = w == -1 && (errno == EAGAIN || errno == EINTR);
            if (sent == input.size() || (w == -1 && !retry)) {
                close(to_child);  // done, or EPIPE: it stopped reading
                to_child = -1;
            }
        }
    }
    if (to_child != -1) {
        close(to_child);
    }
    close(out[0]);
    sigaction(SIGPIPE, &old, nullptr);
    result.status = wait_for(pid);
    return result;
}

int main() {
    Output os = capture({"uname", "-sm"});
    std::cout << "uname -sm printed: " << os.out;

    std::string text;
    while (text.size() < (1 << 20)) {
        text += "hello pipes, this text goes out on one and back on another\n";
    }
    Output up = communicate({"tr", "a-z", "A-Z"}, text);
    std::cout << "sent " << text.size() << " bytes through tr, got " << up.out.size() << " back ("
              << describe(up.status) << "), starting: " << up.out.substr(0, up.out.find('\n') + 1);

    Output head = communicate({"head", "-c", "11"}, text);
    std::cout << "head -c 11 read " << head.out.size() << " bytes of " << text.size() << " and "
              << describe(head.status) << ": " << head.out << '\n';
    return 0;
}
