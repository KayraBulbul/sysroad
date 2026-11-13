// The course's version of this file. You won't need to change it.
#include <cctype>
#include <cstdlib>

#include "shell.hpp"

bool is_valid_name(const std::string& name) {
    if (name.empty() || std::isdigit(static_cast<unsigned char>(name[0]))) {
        return false;
    }
    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return false;
        }
    }
    return true;
}

std::optional<std::string> lookup_var(const Shell& sh, const std::string& name) {
    auto it = sh.vars.find(name);
    if (it != sh.vars.end()) {
        return it->second;
    }
    if (const char* value = std::getenv(name.c_str())) {
        return std::string(value);
    }
    return std::nullopt;
}

void set_var(Shell& sh, const std::string& name, const std::string& value) {
    if (std::getenv(name.c_str()) != nullptr) {
        setenv(name.c_str(), value.c_str(), 1);  // exported: stays exported
    } else {
        sh.vars[name] = value;
    }
}

void export_var(Shell& sh, const std::string& name) {
    auto it = sh.vars.find(name);
    if (it != sh.vars.end()) {
        setenv(name.c_str(), it->second.c_str(), 1);
        sh.vars.erase(it);
    } else if (std::getenv(name.c_str()) == nullptr) {
        setenv(name.c_str(), "", 1);
    }
}

void unset_var(Shell& sh, const std::string& name) {
    sh.vars.erase(name);
    unsetenv(name.c_str());
}
