#include "self.hpp"

#include <dirent.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <string>

extern char** environ;

std::vector<int> open_fds() {
    std::vector<int> fds;
    DIR* dir = opendir("/dev/fd");
    if (dir == nullptr) {
        return fds;
    }
    int own = dirfd(dir);
    while (dirent* entry = readdir(dir)) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        int fd = std::stoi(name);
        if (fd != own) {
            fds.push_back(fd);
        }
    }
    closedir(dir);
    std::sort(fds.begin(), fds.end());
    return fds;
}

std::map<std::string, std::string> environment() {
    std::map<std::string, std::string> env;
    for (char** p = environ; *p != nullptr; ++p) {
        std::string entry = *p;
        auto eq = entry.find('=');
        if (eq != std::string::npos) {
            env[entry.substr(0, eq)] = entry.substr(eq + 1);
        }
    }
    return env;
}

ProcessInfo current_process() {
    ProcessInfo info;
    info.pid = getpid();
    info.ppid = getppid();
    char buf[4096];
    if (getcwd(buf, sizeof buf) != nullptr) {
        info.cwd = buf;
    }
    info.fds = open_fds();
    return info;
}

int main() {
    ProcessInfo me = current_process();
    std::cout << "pid   " << me.pid << '\n';
    std::cout << "ppid  " << me.ppid << '\n';
    std::cout << "cwd   " << me.cwd << '\n';
    std::cout << "fds  ";
    for (int fd : me.fds) {
        std::cout << ' ' << fd;
    }
    std::cout << '\n';
    auto env = environment();
    std::cout << "env   " << env.size() << " variables, HOME=" << env["HOME"] << '\n';
    return 0;
}
