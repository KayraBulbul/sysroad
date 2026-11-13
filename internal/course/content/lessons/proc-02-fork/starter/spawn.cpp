#include "spawn.hpp"

#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <system_error>

pid_t spawn(const std::function<int()>& body) {
    // TODO: flush, fork, and in the child: run body, flush, _exit.
    (void)body;
    return -1;
}

std::vector<int> run_all(int n, const std::function<int(int)>& body) {
    // TODO: spawn all n first, then wait for each.
    (void)n;
    (void)body;
    return {};
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
