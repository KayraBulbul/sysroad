#include "spawn.hpp"

#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <system_error>

pid_t spawn(const std::function<int()>& body) {
    // Anything still in a stdio buffer would be copied into the child and
    // printed twice.
    std::fflush(nullptr);
    pid_t pid = fork();
    if (pid == -1) {
        throw std::system_error(errno, std::generic_category(), "fork");
    }
    if (pid > 0) {
        return pid;
    }
    int code = 1;
    try {
        code = body();
    } catch (...) {
        code = 1;
    }
    std::fflush(nullptr);
    _exit(code);
}

std::vector<int> run_all(int n, const std::function<int(int)>& body) {
    std::vector<pid_t> pids;
    for (int i = 0; i < n; ++i) {
        pids.push_back(spawn([&body, i] { return body(i); }));
    }
    std::vector<int> codes;
    for (pid_t pid : pids) {
        codes.push_back(wait_exit_code(pid));
    }
    return codes;
}

int main() {
    int counter = 100;
    std::cout << "parent " << getpid() << ": counter is " << counter << '\n';

    pid_t child = spawn([&] {
        counter += 1;
        std::cout << "child  " << getpid() << ": my parent is " << getppid() << ", I made counter "
                  << counter << '\n';
        return 7;
    });
    int code = wait_exit_code(child);
    std::cout << "parent: child " << child << " exited " << code << ", and counter is still "
              << counter << '\n';

    auto codes = run_all(4, [](int i) { return i * i; });
    std::cout << "run_all:";
    for (int c : codes) {
        std::cout << ' ' << c;
    }
    std::cout << '\n';
    return 0;
}
