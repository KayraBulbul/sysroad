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

Output capture(const std::vector<std::string>& argv) {
    // TODO: pipe, fork; in the child dup2 the write end onto 1, close both
    // ends, reset SIGPIPE and execvp. The parent closes the write end,
    // reads to the end, and only then waits.
    (void)argv;
    return {};
}

Output communicate(const std::vector<std::string>& argv, const std::string& input) {
    // TODO: two pipes, and a poll loop that writes and reads as each side
    // is ready.
    (void)argv;
    (void)input;
    return {};
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
