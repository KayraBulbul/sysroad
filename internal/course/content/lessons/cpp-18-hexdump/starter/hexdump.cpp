#include "hexdump.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>

std::string hex_line(std::uint64_t offset, std::string_view bytes) {
    // TODO: offset, 16 hex columns in two groups of 8, then |ascii|
    return "";
}

void Dumper::feed(std::string_view bytes) {
    // TODO: collect bytes in pending_, print each full 16-byte line
}

void Dumper::finish() {
    // TODO: the partial last line, then the final offset (if there was any input)
}

int run_hexdump(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
                std::ostream& err) {
    std::vector<std::string_view> files;
    for (std::string_view arg : args) {
        if (arg == "-C") {
            continue;  // canonical output is the only kind there is
        }
        if (arg.starts_with('-') && arg.size() > 1) {
            err << "hexdump: invalid option -- '" << arg[1] << "'\n";
            return 1;
        }
        files.push_back(arg);
    }
    if (files.empty()) {
        files.push_back("-");
    }
    Dumper d(out);
    std::array<char, 4096> buf;
    auto feed_all = [&](std::istream& from) {
        while (from.read(buf.data(), buf.size()) || from.gcount() > 0) {
            d.feed(std::string_view(buf.data(), static_cast<std::size_t>(from.gcount())));
        }
    };
    int status = 0;
    for (std::string_view name : files) {
        if (name == "-") {
            feed_all(in);
            continue;
        }
        std::ifstream f{std::string(name), std::ios::binary};
        if (!f) {
            err << "hexdump: " << name << ": " << std::strerror(errno) << '\n';
            status = 1;
            continue;
        }
        feed_all(f);
    }
    d.finish();
    return status;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_hexdump(args, std::cin, std::cout, std::cerr);
}
