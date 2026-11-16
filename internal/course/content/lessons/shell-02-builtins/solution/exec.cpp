#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "conch.hpp"

int run_line(Shell& sh, const std::string& line) {
    std::vector<std::string> words = split_words(line);
    if (words.empty()) {
        return sh.last_status;  // a blank line or a comment runs nothing
    }
    if (Builtin builtin = find_builtin(words[0])) {
        sh.last_status = builtin(sh, words);  // in the shell itself
        std::cout.flush();
    } else {
        sh.last_status = run_command(words);
    }
    return sh.last_status;
}

// $? for a status from waitpid: the exit code, or 128 plus the signal.
static int shell_status(int wstatus) {
    if (WIFSIGNALED(wstatus)) {
        return 128 + WTERMSIG(wstatus);
    }
    return WEXITSTATUS(wstatus);
}

int run_command(const std::vector<std::string>& argv) {
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
        execvp(args[0], args.data());
        int err = errno;
        if (err == ENOENT) {
            std::cerr << "conch: " << argv[0] << ": command not found\n";
            _exit(127);
        }
        std::cerr << "conch: " << argv[0] << ": " << std::strerror(err) << '\n';
        _exit(126);
    }
    int wstatus = 0;
    while (waitpid(pid, &wstatus, 0) == -1) {
        if (errno != EINTR) {
            std::cerr << "conch: waitpid: " << std::strerror(errno) << '\n';
            return 1;
        }
    }
    return shell_status(wstatus);
}
