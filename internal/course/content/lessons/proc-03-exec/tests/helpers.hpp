#pragma once

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

inline void write_file(const std::string& path, const std::string& data, mode_t mode) {
    std::ofstream(path, std::ios::binary) << data;
    ::chmod(path.c_str(), mode);
}

inline std::string slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// Runs exec_with in a child; returns the child's exit code (127 if
// exec_with returned).
template <class F> int in_child(F f) {
    pid_t pid = ::fork();
    if (pid == 0) {
        f();
        ::_exit(127);
    }
    int status = 0;
    ::waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
