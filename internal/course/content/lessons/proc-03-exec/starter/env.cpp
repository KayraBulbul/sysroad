#include "env.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

EnvCommand parse_env_args(const std::vector<std::string>& args) {
    EnvCommand cmd;
    (void)args;  // TODO
    return cmd;
}

std::vector<std::string> build_environment(const std::vector<std::string>& current,
                                           const EnvCommand& cmd) {
    (void)current;  // TODO
    (void)cmd;
    return {};
}

std::string find_in_path(const std::string& name, const std::string& path) {
    (void)name;  // TODO
    (void)path;
    return "";
}

void exec_with(const std::vector<std::string>& argv, const std::vector<std::string>& env) {
    // TODO: build the two null-terminated char* arrays, find the program
    // and execve it.
    (void)argv;
    (void)env;
    errno = ENOENT;
}

extern char** environ;

int main(int argc, char* argv[]) {
    EnvCommand cmd;
    try {
        cmd = parse_env_args(std::vector<std::string>(argv + 1, argv + argc));
    } catch (const std::invalid_argument& e) {
        std::cerr << "env: " << e.what() << "\nusage: env [-i] [-u NAME]... [NAME=VALUE]... "
                  << "[COMMAND [ARG]...]\n";
        return 125;
    }
    std::vector<std::string> current;
    for (char** p = environ; *p != nullptr; ++p) {
        current.push_back(*p);
    }
    std::vector<std::string> env = build_environment(current, cmd);
    if (cmd.argv.empty()) {
        for (const auto& entry : env) {
            std::cout << entry << '\n';
        }
        return 0;
    }
    std::cout.flush();
    exec_with(cmd.argv, env);
    int err = errno;
    std::cerr << "env: " << cmd.argv[0] << ": " << std::strerror(err) << '\n';
    return err == ENOENT ? 127 : 126;
}
