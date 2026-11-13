#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// env [-i] [-u NAME]... [NAME=VALUE]... [COMMAND [ARG]...]
struct EnvCommand {
    bool clear = false;                                     // -i
    std::vector<std::string> unset;                         // -u NAME
    std::vector<std::pair<std::string, std::string>> set;  // NAME=VALUE
    std::vector<std::string> argv;                          // COMMAND [ARG]...
};

// Parses env's arguments (without argv[0]). Throws std::invalid_argument
// for an unknown option or a -u with no name.
EnvCommand parse_env_args(const std::vector<std::string>& args);

// The new environment: `current` (or nothing, with -i), minus the -u
// names, with each NAME=VALUE replacing NAME where it is or added at the
// end.
std::vector<std::string> build_environment(const std::vector<std::string>& current,
                                           const EnvCommand& cmd);

// Where exec would find `name`: `name` itself if it contains a '/',
// else the first executable regular file in path's folders. "" if none.
std::string find_in_path(const std::string& name, const std::string& path);

// Runs argv[0] (found through PATH from `env`) with this environment, in
// place of this process. Only returns if that fails, with errno set.
void exec_with(const std::vector<std::string>& argv, const std::vector<std::string>& env);
