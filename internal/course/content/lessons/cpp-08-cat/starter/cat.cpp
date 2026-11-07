#include "cat.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

void cat_lines(std::istream& in, std::ostream& out) {
    // TODO: copy with std::getline
}

int run_cat(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
            std::ostream& err) {
    // TODO: stdin or each file; report missing files and directories
    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_cat(args, std::cin, std::cout, std::cerr);
}
