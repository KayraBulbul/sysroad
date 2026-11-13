// The course's version of this file. You won't need to change it.
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <system_error>

#include "shell.hpp"

void run_line(Shell& sh, const std::string& line) {
    List list;
    try {
        list = parse(tokenize(line));
    } catch (const SyntaxError& e) {
        std::cerr << "conch: syntax error: " << e.what() << '\n';
        sh.last_status = 2;
        return;
    }
    try {
        run_list(sh, list);
    } catch (const SyntaxError& e) {
        std::cerr << "conch: syntax error: " << e.what() << '\n';
        sh.last_status = 2;
    }
}

[[noreturn]] void exec_command(Shell& sh, const Command& c) {
    for (const auto& [name, value] : c.assignments) {
        setenv(name.c_str(), value.c_str(), 1);
    }
    try {
        apply_redirects(c.redirects);
    } catch (const std::system_error& e) {
        std::cerr << "conch: " << e.what() << '\n';
        _exit(1);
    }
    if (c.argv.empty()) {
        _exit(0);
    }
    if (Builtin b = find_builtin(c.argv[0])) {
        int status = b(sh, c.argv);
        std::cout.flush();
        std::fflush(nullptr);
        _exit(status);
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

int run_in_shell(Shell& sh, const Command& c) {
    if (c.argv.empty()) {
        for (const auto& [name, value] : c.assignments) {
            set_var(sh, name, value);
        }
    }
    SavedFds saved(c.redirects);
    try {
        apply_redirects(c.redirects);
    } catch (const std::system_error& e) {
        std::cerr << "conch: " << e.what() << '\n';
        return 1;
    }
    if (c.argv.empty()) {
        return 0;
    }
    int status = find_builtin(c.argv[0])(sh, c.argv);
    std::cout.flush();  // before the fds go back
    std::fflush(nullptr);
    return status;
}
