#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <sys/types.h>

#include <map>
#include <string>
#include <vector>

struct ProcessInfo {
    pid_t pid = 0;
    pid_t ppid = 0;
    std::string cwd;
    std::vector<int> fds;
};

// The fds this process has open, sorted, as /dev/fd lists them.
std::vector<int> open_fds();

// The environment as NAME -> value.
std::map<std::string, std::string> environment();

// This process's PID, parent's PID, current directory and open fds.
ProcessInfo current_process();
