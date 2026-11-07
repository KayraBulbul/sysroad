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
    std::size_t matched = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.find(pattern) != std::string::npos) {
            out << prefix << line << '\n';
            ++matched;
        }
    }
    return matched;
}

int run_grep(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
             std::ostream& err) {
    if (args.empty()) {
        err << "Usage: grep [OPTION]... PATTERNS [FILE]...\n";
        return 2;
    }
    std::string_view pattern = args[0];
    std::vector<std::string_view> files(args.begin() + 1, args.end());
    if (files.empty()) {
        files.push_back("-");
    }
    bool names = files.size() > 1;
    bool matched = false, error = false;
    for (std::string_view name : files) {
        if (name == "-") {
            matched |= grep_stream(in, pattern, names ? "(standard input):" : "", out) > 0;
            continue;
        }
        std::error_code ec;
        if (fs::is_directory(fs::path(name), ec)) {
            err << "grep: " << name << ": Is a directory\n";
            error = true;
            continue;
        }
        std::ifstream f{std::string(name), std::ios::binary};
        if (!f) {
            err << "grep: " << name << ": " << std::strerror(errno) << '\n';
            error = true;
            continue;
        }
        std::string prefix = names ? std::string(name) + ":" : "";
        matched |= grep_stream(f, pattern, prefix, out) > 0;
    }
    if (error) {
        return 2;
    }
    return matched ? 0 : 1;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_grep(args, std::cin, std::cout, std::cerr);
}
