#include "self.hpp"

#include <dirent.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <string>

extern char** environ;

std::vector<int> open_fds() {
    return {};  // TODO: opendir("/dev/fd"), readdir, closedir
}

std::map<std::string, std::string> environment() {
    return {};  // TODO: walk environ until its null pointer
}

ProcessInfo current_process() {
    return {};  // TODO
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
