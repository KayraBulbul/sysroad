#pragma once

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

inline void write_file(const std::string& path, const std::string& data) {
    std::ofstream(path, std::ios::binary) << data;
}

inline std::string slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

inline int lowest_free_fd() {
    int fd = ::open("/dev/null", O_RDONLY);
    ::close(fd);
    return fd;
}

inline std::set<int> inherited_fds() {
    std::set<int> fds;
    for (int fd = 0; fd < 64; ++fd) {
        int flags = ::fcntl(fd, F_GETFD);
        if (flags != -1 && !(flags & FD_CLOEXEC)) {
            fds.insert(fd);
        }
    }
    return fds;
}

// Points one of this process's fds at a file for as long as it lives.
struct Redirected {
    int fd, saved;
    Redirected(int fd, const char* path, int flags) : fd(fd) {
        std::fflush(nullptr);
        saved = ::dup(fd);
        int f = ::open(path, flags, 0644);
        ::dup2(f, fd);
        ::close(f);
    }
    ~Redirected() {
        std::fflush(nullptr);
        ::dup2(saved, fd);
        ::close(saved);
    }
};

// Runs f in a child that SIGALRM kills after `seconds`. Returns f's
// result, or -1 if it hung.
template <class F> int within(unsigned seconds, F f) {
    std::fflush(nullptr);
    pid_t pid = ::fork();
    if (pid == 0) {
        ::alarm(seconds);
        int r = f();
        ::_exit(r);
    }
    int st = 0;
    ::waitpid(pid, &st, 0);
    return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}
