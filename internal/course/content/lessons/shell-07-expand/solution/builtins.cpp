// The course's version of this file. You won't need to change it.
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

#include "shell.hpp"

extern char** environ;

static int builtin_cd(Shell& sh, const std::vector<std::string>& argv) {
    std::string dir;
    bool show = false;
    if (argv.size() > 2) {
        std::cerr << "conch: cd: too many arguments\n";
        return 1;
    }
    if (argv.size() == 1) {
        const char* home = std::getenv("HOME");
        if (home == nullptr) {
            std::cerr << "conch: cd: HOME not set\n";
            return 1;
        }
        dir = home;
    } else if (argv[1] == "-") {
        const char* old = std::getenv("OLDPWD");
        if (old == nullptr) {
            std::cerr << "conch: cd: OLDPWD not set\n";
            return 1;
        }
        dir = old;
        show = true;
    } else {
        dir = argv[1];
    }
    char before[PATH_MAX];
    bool have_before = getcwd(before, sizeof before) != nullptr;
    if (chdir(dir.c_str()) == -1) {
        std::cerr << "conch: cd: " << dir << ": " << std::strerror(errno) << '\n';
        return 1;
    }
    if (have_before) {
        setenv("OLDPWD", before, 1);
    }
    char now[PATH_MAX];
    if (getcwd(now, sizeof now) != nullptr) {
        setenv("PWD", now, 1);
        if (show) {
            std::cout << now << '\n';
        }
    }
    (void)sh;
    return 0;
}

static int builtin_pwd(Shell&, const std::vector<std::string>&) {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof buf) == nullptr) {
        std::cerr << "conch: pwd: " << std::strerror(errno) << '\n';
        return 1;
    }
    std::cout << buf << '\n';
    return 0;
}

static int builtin_exit(Shell& sh, const std::vector<std::string>& argv) {
    if (argv.size() > 2) {
        std::cerr << "conch: exit: too many arguments\n";
        return 1;
    }
    int code = sh.last_status;
    if (argv.size() == 2) {
        char* end = nullptr;
        long n = std::strtol(argv[1].c_str(), &end, 10);
        if (argv[1].empty() || *end != '\0') {
            std::cerr << "conch: exit: " << argv[1] << ": numeric argument required\n";
            n = 2;
        }
        code = static_cast<int>(n & 0xff);
    }
    if (!may_exit(sh)) {
        return 1;
    }
    sh.exiting = true;
    sh.exit_code = code;
    return code;
}

static int builtin_export(Shell& sh, const std::vector<std::string>& argv) {
    if (argv.size() == 1) {
        for (char** p = environ; *p != nullptr; ++p) {
            std::cout << "export " << *p << '\n';
        }
        return 0;
    }
    int status = 0;
    for (size_t i = 1; i < argv.size(); ++i) {
        size_t eq = argv[i].find('=');
        std::string name = argv[i].substr(0, eq);
        if (!is_valid_name(name)) {
            std::cerr << "conch: export: " << argv[i] << ": not a valid name\n";
            status = 1;
            continue;
        }
        if (eq != std::string::npos) {
            set_var(sh, name, argv[i].substr(eq + 1));
        }
        export_var(sh, name);
    }
    return status;
}

static int builtin_unset(Shell& sh, const std::vector<std::string>& argv) {
    for (size_t i = 1; i < argv.size(); ++i) {
        unset_var(sh, argv[i]);
    }
    return 0;
}

static std::string find_in_path(const std::string& name) {
    const char* path = std::getenv("PATH");
    std::string p = path != nullptr ? path : "/usr/bin:/bin";
    size_t start = 0;
    for (;;) {
        size_t colon = p.find(':', start);
        std::string dir = p.substr(start, colon - start);
        std::string candidate = (dir.empty() ? "." : dir) + "/" + name;
        struct stat st;
        if (stat(candidate.c_str(), &st) == 0 && S_ISREG(st.st_mode) &&
            access(candidate.c_str(), X_OK) == 0) {
            return candidate;
        }
        if (colon == std::string::npos) {
            return "";
        }
        start = colon + 1;
    }
}

static int builtin_type(Shell&, const std::vector<std::string>& argv) {
    int status = 0;
    for (size_t i = 1; i < argv.size(); ++i) {
        const std::string& name = argv[i];
        if (find_builtin(name) != nullptr) {
            std::cout << name << " is a shell builtin\n";
            continue;
        }
        std::string path = name.find('/') == std::string::npos ? find_in_path(name) : "";
        if (!path.empty()) {
            std::cout << name << " is " << path << '\n';
        } else {
            std::cerr << "conch: type: " << name << ": not found\n";
            status = 1;
        }
    }
    return status;
}

Builtin find_builtin(const std::string& name) {
    static const std::map<std::string, Builtin> builtins = {
        {"cd", builtin_cd},         {"pwd", builtin_pwd},     {"exit", builtin_exit},
        {"export", builtin_export}, {"unset", builtin_unset}, {"type", builtin_type},
        {"jobs", builtin_jobs},     {"wait", builtin_wait},   {"fg", builtin_fg},
        {"bg", builtin_bg},
    };
    auto it = builtins.find(name);
    return it == builtins.end() ? nullptr : it->second;
}
