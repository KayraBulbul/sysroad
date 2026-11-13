#include "env.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

EnvCommand parse_env_args(const std::vector<std::string>& args) {
    EnvCommand cmd;
    size_t i = 0;
    for (; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-i" || a == "-") {
            cmd.clear = true;
        } else if (a == "-u") {
            if (i + 1 == args.size()) {
                throw std::invalid_argument("-u needs a name");
            }
            cmd.unset.push_back(args[++i]);
        } else if (a == "--") {
            ++i;
            break;
        } else if (a.size() > 1 && a[0] == '-') {
            throw std::invalid_argument("unknown option " + a);
        } else {
            break;
        }
    }
    for (; i < args.size(); ++i) {
        auto eq = args[i].find('=');
        if (eq == std::string::npos || eq == 0) {
            break;
        }
        cmd.set.emplace_back(args[i].substr(0, eq), args[i].substr(eq + 1));
    }
    cmd.argv.assign(args.begin() + i, args.end());
    return cmd;
}

static std::string name_of(const std::string& entry) {
    return entry.substr(0, entry.find('='));
}

std::vector<std::string> build_environment(const std::vector<std::string>& current,
                                           const EnvCommand& cmd) {
    std::vector<std::string> env;
    if (!cmd.clear) {
        for (const auto& entry : current) {
            bool removed = false;
            for (const auto& name : cmd.unset) {
                removed = removed || name_of(entry) == name;
            }
            if (!removed) {
                env.push_back(entry);
            }
        }
    }
    for (const auto& [name, value] : cmd.set) {
        std::string entry = name + "=" + value;
        bool replaced = false;
        for (auto& existing : env) {
            if (name_of(existing) == name) {
                existing = entry;
                replaced = true;
            }
        }
        if (!replaced) {
            env.push_back(entry);
        }
    }
    return env;
}

static bool is_executable_file(const std::string& p) {
    struct stat st;
    return ::stat(p.c_str(), &st) == 0 && S_ISREG(st.st_mode) && ::access(p.c_str(), X_OK) == 0;
}

std::string find_in_path(const std::string& name, const std::string& path) {
    if (name.find('/') != std::string::npos) {
        return name;
    }
    if (name.empty()) {
        return "";
    }
    size_t start = 0;
    for (;;) {
        size_t colon = path.find(':', start);
        std::string dir = path.substr(start, colon - start);
        if (dir.empty()) {
            dir = ".";  // an empty entry means the current folder
        }
        std::string candidate = dir + "/" + name;
        if (is_executable_file(candidate)) {
            return candidate;
        }
        if (colon == std::string::npos) {
            return "";
        }
        start = colon + 1;
    }
}

static std::vector<char*> c_strings(const std::vector<std::string>& v) {
    std::vector<char*> out;
    for (const auto& s : v) {
        out.push_back(const_cast<char*>(s.c_str()));
    }
    out.push_back(nullptr);
    return out;
}

void exec_with(const std::vector<std::string>& argv, const std::vector<std::string>& env) {
    if (argv.empty()) {
        errno = ENOENT;
        return;
    }
    std::string path = "/usr/bin:/bin";
    for (const auto& entry : env) {
        if (entry.rfind("PATH=", 0) == 0) {
            path = entry.substr(5);
        }
    }
    std::string program = find_in_path(argv[0], path);
    if (program.empty()) {
        errno = ENOENT;
        return;
    }
    std::vector<char*> args = c_strings(argv);
    std::vector<char*> envp = c_strings(env);
    ::execve(program.c_str(), args.data(), envp.data());
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
