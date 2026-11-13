#include <sys/stat.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

#include "shell.hpp"

extern char** environ;

// TODO: cd, pwd, exit, export, unset and type, each as
// static int builtin_NAME(Shell& sh, const std::vector<std::string>& argv)

Builtin find_builtin(const std::string& name) {
    (void)name;  // TODO: look the name up in a table of your builtins
    return nullptr;
}
