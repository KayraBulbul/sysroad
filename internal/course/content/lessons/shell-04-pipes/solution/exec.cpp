#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <optional>
#include <system_error>

#include "conch.hpp"

int run_line(Shell& sh, const std::string& line) {
    List list;
    try {
        list = parse(tokenize(line));
    } catch (const SyntaxError& e) {
        std::cerr << "conch: syntax error: " << e.what() << '\n';
        return sh.last_status = 2;
    }
    return run_list(sh, list);  // nothing at all for a blank line or a comment
}

int shell_status(int wstatus) {
    if (WIFSIGNALED(wstatus)) {
        return 128 + WTERMSIG(wstatus);
    }
    return WEXITSTATUS(wstatus);
}

int run_builtin(Shell& sh, Builtin builtin, const Command& c) {
    std::optional<SavedFds> saved;
    try {
        saved.emplace(c.redirects);
        apply_redirects(c.redirects);
    } catch (const std::system_error& e) {
        std::cerr << "conch: " << e.what() << '\n';
        return 1;  // saved puts back whatever was applied
    }
    int status = builtin(sh, c.argv);
    std::cout.flush();  // before the fds go back
    std::fflush(nullptr);
    return status;
}

[[noreturn]] void exec_command(Shell& sh, const Command& c) {
    try {
        apply_redirects(c.redirects);
    } catch (const std::system_error& e) {
        std::cerr << "conch: " << e.what() << '\n';
        _exit(1);
    }
    if (c.argv.empty()) {
        _exit(0);  // only redirections: "> file" makes the file
    }
    if (Builtin builtin = find_builtin(c.argv[0])) {
        int status = builtin(sh, c.argv);  // in this child, in a pipeline
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
