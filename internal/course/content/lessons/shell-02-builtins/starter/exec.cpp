// The course's version of this file. You won't need to change it.
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "shell.hpp"

void run_line(Shell& sh, const std::string& line) {
    std::vector<std::string> words = split_words(line);
    if (words.empty()) {
        return;  // a blank line or a comment
    }
    sh.last_status = run_command(sh, words);
}

int run_command(Shell& sh, const std::vector<std::string>& argv) {
    if (Builtin b = find_builtin(argv[0])) {
        int status = b(sh, argv);
        std::cout.flush();
        return status;
    }
    std::vector<char*> args;
    for (const auto& a : argv) {
        args.push_back(const_cast<char*>(a.c_str()));
    }
    args.push_back(nullptr);
    std::fflush(nullptr);
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "conch: fork: " << std::strerror(errno) << '\n';
        return 1;
    }
    if (pid == 0) {
        signal(SIGPIPE, SIG_DFL);
        execvp(args[0], args.data());
        int err = errno;
        if (err == ENOENT) {
            std::cerr << "conch: " << argv[0] << ": command not found\n";
            _exit(127);
        }
        std::cerr << "conch: " << argv[0] << ": " << std::strerror(err) << '\n';
        _exit(126);
    }
    return shell_status(wait_for(pid));
}
