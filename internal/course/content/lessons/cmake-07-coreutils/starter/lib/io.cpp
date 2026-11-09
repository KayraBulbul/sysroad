// You won't need to change this file.
#include "coreutils/io.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace coreutils {

int for_each_input(std::string_view tool, std::span<char* const> files,
                   const std::function<void(std::istream& in, std::string_view name)>& each) {
    if (files.empty()) {
        each(std::cin, "-");
        return 0;
    }
    int status = 0;
    for (const char* name : files) {
        if (std::string_view(name) == "-") {
            each(std::cin, "-");
            continue;
        }
        std::ifstream in(name, std::ios::binary);
        if (!in) {
            std::cerr << tool << ": " << name << ": can't open\n";
            status = 1;
            continue;
        }
        each(in, name);
    }
    return status;
}

}  // namespace coreutils
