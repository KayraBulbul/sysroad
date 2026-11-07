#include "cat.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

std::size_t copy_stream(std::istream& in, std::ostream& out) {
    std::array<char, 65536> buf;
    std::size_t total = 0;
    while (in.read(buf.data(), buf.size()) || in.gcount() > 0) {
        out.write(buf.data(), in.gcount());
        total += static_cast<std::size_t>(in.gcount());
    }
    return total;
}

int run_cat(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
            std::ostream& err) {
    std::vector<std::string_view> files = args;
    if (files.empty()) {
        files.push_back("-");
    }
    int status = 0;
    for (std::string_view name : files) {
        if (name == "-") {
            copy_stream(in, out);
            continue;
        }
        std::error_code ec;
        if (fs::is_directory(fs::path(name), ec)) {
            err << "cat: " << name << ": Is a directory\n";
            status = 1;
            continue;
        }
        std::ifstream f{std::string(name), std::ios::binary};
        if (!f) {
            err << "cat: " << name << ": " << std::strerror(errno) << '\n';
            status = 1;
            continue;
        }
        copy_stream(f, out);
    }
    return status;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_cat(args, std::cin, std::cout, std::cerr);
}
