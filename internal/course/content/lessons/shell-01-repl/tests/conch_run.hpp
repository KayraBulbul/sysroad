#pragma once
// Runs conch the ways the tests need: your whole program reading a script,
// as `conch < script` would, or one line in a Shell of the test's own. Each
// gives back what it printed on stdout and stderr, and its status.

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <climits>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>

#include "conch.hpp"

// Your main, renamed for the tests: conch's is int main().
int sysroad_student_main();

namespace shtest {

// The folder the tests started in: their files stay there, whatever cd does.
inline const std::string& test_dir() {
    static const std::string dir = [] {
        char buf[PATH_MAX];
        return std::string(getcwd(buf, sizeof buf) != nullptr ? buf : ".");
    }();
    return dir;
}

inline std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}

inline void write_file(const std::string& path, const std::string& text) {
    std::ofstream(path, std::ios::binary | std::ios::trunc) << text;
}

// Whether part is somewhere in s.
inline bool contains(const std::string& s, const std::string& part) { return s.find(part) != std::string::npos; }

// The lowest fd not in use: if a shell leaks fds, it climbs.
inline int lowest_free_fd() {
    int fd = open("/dev/null", O_RDONLY);
    close(fd);
    return fd;
}

struct Ran {
    std::string out;
    std::string err;
    int status = 0;  // the exit status; -1 if it was killed or didn't finish
};

// Runs conch's main with script as its input (a file, not a terminal) in a
// process of its own. setup runs in that process first, for tests that
// need, say, a lower limit on open files.
inline Ran conch(const std::string& script, const std::function<void()>& setup = {}, int timeout_ms = 5000) {
    static int runs = 0;
    test_dir();  // before anything moves
    std::string base = test_dir() + "/conch-run-" + std::to_string(++runs);
    write_file(base + ".in", script);
    std::fflush(nullptr);
    pid_t pid = fork();
    if (pid == 0) {
        int in = open((base + ".in").c_str(), O_RDONLY);
        int out = open((base + ".out").c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        int err = open((base + ".err").c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(in, 0);
        dup2(out, 1);
        dup2(err, 2);
        close(in);
        close(out);
        close(err);
        if (setup) {
            setup();
        }
        int r = sysroad_student_main();
        std::fflush(nullptr);
        _exit(r);
    }
    Ran ran;
    auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    int st = 0;
    while (waitpid(pid, &st, WNOHANG) == 0) {
        if (std::chrono::steady_clock::now() > until) {
            kill(pid, SIGKILL);
            waitpid(pid, &st, 0);
            st = -1;
            break;
        }
        usleep(5'000);
    }
    ran.status = st != -1 && WIFEXITED(st) ? WEXITSTATUS(st) : -1;
    ran.out = read_file(base + ".out");
    ran.err = read_file(base + ".err");
    return ran;
}

// Runs one line with run_line in sh, inside the tests' own process, with
// stdout and stderr caught in files. status is what run_line returned.
inline Ran in_shell(Shell& sh, const std::string& line) {
    const std::string base = test_dir() + "/in-shell";
    std::fflush(nullptr);
    int saved1 = dup(1), saved2 = dup(2);
    int out = open((base + ".out").c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int err = open((base + ".err").c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(out, 1);
    dup2(err, 2);
    close(out);
    close(err);
    Ran ran;
    ran.status = run_line(sh, line);
    std::cout.flush();
    std::fflush(nullptr);
    dup2(saved1, 1);
    dup2(saved2, 2);
    close(saved1);
    close(saved2);
    ran.out = read_file(base + ".out");
    ran.err = read_file(base + ".err");
    return ran;
}

}  // namespace shtest
