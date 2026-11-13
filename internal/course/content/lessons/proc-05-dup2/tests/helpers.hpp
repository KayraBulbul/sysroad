#pragma once

#include <fcntl.h>
#include <unistd.h>

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

inline bool exists(const std::string& path) {
    return ::access(path.c_str(), F_OK) == 0;
}

// The fds open in this process, and inherited by children, below 64.
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

// Points this process's stdout at a file for as long as it lives.
struct CaptureStdout {
    int saved;
    explicit CaptureStdout(const char* path) {
        std::fflush(nullptr);
        saved = ::dup(1);
        int fd = ::open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        ::dup2(fd, 1);
        ::close(fd);
    }
    ~CaptureStdout() {
        std::fflush(nullptr);
        ::dup2(saved, 1);
        ::close(saved);
    }
};
