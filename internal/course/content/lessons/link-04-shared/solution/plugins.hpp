#pragma once
// Builds small plugins with the C++ compiler, for main.cpp and the tests.
// You won't need to change this file.

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>

#include <unistd.h>

// A folder of its own in the temporary directory, removed when the
// program exits.
inline std::filesystem::path plugin_dir() {
    struct Dir {
        std::filesystem::path path;
        Dir()
            : path(std::filesystem::temp_directory_path() /
                   ("sysroad-plugins-" + std::to_string(getpid()))) {
            std::filesystem::create_directories(path);
        }
        ~Dir() {
            std::error_code ec;
            std::filesystem::remove_all(path, ec);
        }
    };
    static Dir dir;
    return dir.path;
}

// Compiles source into a shared library called name.so in plugin_dir(),
// and returns its path. Each name is built once per run: rewriting a
// library that's loaded could crash the program using it.
inline std::string build_plugin(const std::string& name, const std::string& source) {
    static std::map<std::string, std::string> built;
    if (auto it = built.find(name); it != built.end()) {
        return it->second;
    }
    auto dir = plugin_dir();
    auto src = dir / (name + ".cpp");
    auto lib = dir / (name + ".so");
    std::ofstream(src) << source;
    const char* cxx = std::getenv("SYSROAD_CXX");
    std::string cmd = std::string(cxx ? cxx : "c++") + " -std=c++20 -shared -fPIC -o '" +
                      lib.string() + "' '" + src.string() + "'";
    if (std::system(cmd.c_str()) != 0) {
        throw std::runtime_error("couldn't build " + name + ": " + cmd);
    }
    return built[name] = lib.string();
}

// Two plugins with the same interface: a name, and a function that
// changes a string in place.
inline const char* shout_source = R"(
#include <cctype>
#include <cstring>
extern "C" const char* plugin_name() { return "shout"; }
extern "C" void plugin_apply(char* s) {
    for (; *s; ++s) *s = static_cast<char>(std::toupper(static_cast<unsigned char>(*s)));
}
)";

inline const char* reverse_source = R"(
#include <algorithm>
#include <cstring>
extern "C" const char* plugin_name() { return "reverse"; }
extern "C" void plugin_apply(char* s) { std::reverse(s, s + std::strlen(s)); }
int plugin_version = 3;
)";
