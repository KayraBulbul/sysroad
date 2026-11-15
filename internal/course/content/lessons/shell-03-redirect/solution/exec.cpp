#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

#include "conch.hpp"

int run_line(Shell& sh, const std::string& line) {
    Command c;
    try {
        c = parse_command(tokenize(line));
    } catch (const SyntaxError& e) {
        std::cerr << "conch: syntax error: " << e.what() << '\n';
        return sh.last_status = 2;
    }
    if (c.argv.empty() && c.redirects.empty()) {
        return sh.last_status;  // a blank line or a comment runs nothing
    }
    Builtin builtin = c.argv.empty() ? nullptr : find_builtin(c.argv[0]);
    if (builtin != nullptr) {
        sh.last_status = builtin(sh, c.argv);  // in the shell itself
        std::cout.flush();
    } else {
        sh.last_status = run_command(c);
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

// In the child: the redirections, then the program. Never returns.
[[noreturn]] static void exec_command(const Command& c) {
    try {
        apply_redirects(c.redirects);
    } catch (const std::system_error& e) {
        std::cerr << "conch: " << e.what() << '\n';
        _exit(1);
    }
    if (c.argv.empty()) {
        _exit(0);  // only redirections: "> file" makes the file
    }
    std::vector<char*> args;
    for (const auto& a : c.argv) {
        args.push_back(const_cast<char*>(a.c_str()));
    }
    args.push_back(nullptr);
    execvp(args[0], args.data());
    int err = errno;
    if (err == ENOENT) {
        std::cerr << "conch: " << c.argv[0] << ": command not found\n";
        _exit(127);
    }
    std::cerr << "conch: " << c.argv[0] << ": " << std::strerror(err) << '\n';
    _exit(126);
}

int run_command(const Command& c) {
    std::fflush(nullptr);
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "conch: fork: " << std::strerror(errno) << '\n';
        return 1;
    }
    if (pid == 0) {
        exec_command(c);
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
