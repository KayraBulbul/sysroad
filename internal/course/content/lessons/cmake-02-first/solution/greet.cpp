// You won't need to change this file.
#include "greet.hpp"

std::string greet(std::string_view name) {
    // starts_with and ends_with are new in C++20. Built as C++17, this
    // file doesn't compile.
    if (name.empty() || name.starts_with("--")) {
        return "Hello, stranger!";
    }
    if (name.ends_with("?")) {
        return "Hello, " + std::string(name.substr(0, name.size() - 1)) + "?";
    }
    return "Hello, " + std::string(name) + "!";
}
