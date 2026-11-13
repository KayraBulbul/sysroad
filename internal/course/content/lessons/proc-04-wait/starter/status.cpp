#include "status.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <system_error>

pid_t start(const std::vector<std::string>& argv) {
    // TODO: build a char* array, flush, fork; the child execvp's, and
    // _exit(127)s if that fails.
    (void)argv;
    return -1;
}

Status decode(int wstatus) {
    (void)wstatus;  // TODO: WIFEXITED, WEXITSTATUS, WIFSIGNALED, WTERMSIG, WCOREDUMP
    return {};
}

Status wait_for(pid_t pid) {
    (void)pid;  // TODO
    return {};
}

std::string signal_name(int sig) {
    return "signal " + std::to_string(sig);  // TODO
}

std::string describe(const Status& s) {
    (void)s;  // TODO
    return "";
}

int shell_status(const Status& s) {
    (void)s;  // TODO
    return 0;
}

std::vector<std::pair<pid_t, Status>> reap_finished() {
    return {};  // TODO: waitpid(-1, ..., WNOHANG) until it has nothing more
}

// retry [-n TIMES] COMMAND [ARG]...: runs COMMAND until it succeeds, at
// most TIMES times (3 by default), and exits the way its last run did.
int main(int argc, char* argv[]) {
    int times = 3;
    int first = 1;
    if (argc > 2 && std::string(argv[1]) == "-n") {
        times = std::atoi(argv[2]);
        first = 3;
    }
    if (first >= argc || times < 1) {
        std::cerr << "usage: retry [-n TIMES] COMMAND [ARG]...\n";
        return 2;
    }
    std::vector<std::string> command(argv + first, argv + argc);
    Status s;
    for (int attempt = 1; attempt <= times; ++attempt) {
        std::cout.flush();
        s = wait_for(start(command));
        std::cout << "retry: attempt " << attempt << " of " << times << ": " << describe(s) << '\n';
        if (s.ok()) {
            break;
        }
    }
    return shell_status(s);
}
