#include "grep.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

std::size_t grep_stream(std::istream& in, std::string_view pattern, std::string_view prefix,
                        std::ostream& out) {
    // TODO: print each line containing pattern, after prefix
    return 0;
}

int run_grep(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
             std::ostream& err) {
    // TODO: the pattern, then stdin or each file; names when there's more than one;
    // errors for missing files and directories; the exit status
    return 1;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_grep(args, std::cin, std::cout, std::cerr);
}
