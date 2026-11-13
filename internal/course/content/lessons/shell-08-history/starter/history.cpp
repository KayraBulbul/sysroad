#include <cstdio>
#include <fstream>
#include <iostream>

#include "shell.hpp"

void add_history(Shell& sh, const std::string& line) {
    (void)sh;  // TODO
    (void)line;
}

int builtin_history(Shell& sh, const std::vector<std::string>& argv) {
    (void)sh;  // TODO
    (void)argv;
    return 0;
}

void load_history(Shell& sh, const std::string& path) {
    (void)sh;  // TODO
    (void)path;
}

void save_history(const Shell& sh, const std::string& path) {
    (void)sh;  // TODO
    (void)path;
}
