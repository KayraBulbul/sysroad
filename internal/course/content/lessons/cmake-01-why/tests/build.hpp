#pragma once
// Builds the test projects. Each rule's command is "make TARGET".
#include <initializer_list>
#include <string>
#include <vector>

#include "minimake.hpp"

inline Rules rules(std::initializer_list<std::pair<std::string, std::vector<std::string>>> list) {
    Rules out;
    for (const auto& [target, deps] : list) {
        out[target] = Rule{target, deps, "make " + target};
    }
    return out;
}

// app <- main.o, greet.o; main.o <- main.cpp, greet.hpp; greet.o <- greet.cpp, greet.hpp
inline Rules hello_project() {
    return rules({
        {"app", {"main.o", "greet.o"}},
        {"main.o", {"main.cpp", "greet.hpp"}},
        {"greet.o", {"greet.cpp", "greet.hpp"}},
    });
}

inline std::vector<std::string> cmds(std::initializer_list<const char*> list) {
    return {list.begin(), list.end()};
}
