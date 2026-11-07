#include "hexdump.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>

std::string hex_line(std::uint64_t offset, std::string_view bytes) {
    std::string s = std::format("{:08x} ", offset);
    for (std::size_t i = 0; i < 16; ++i) {
        if (i % 8 == 0) {
            s += ' ';
        }
        if (i < bytes.size()) {
            s += std::format("{:02x} ", static_cast<unsigned char>(bytes[i]));
        } else {
            s += "   ";
        }
    }
    s += " |";
    for (char ch : bytes) {
        s += (ch >= 0x20 && ch < 0x7f) ? ch : '.';
    }
    return s + "|\n";
}

void Dumper::feed(std::string_view bytes) {
    pending_ += bytes;
    std::size_t done = 0;
    while (pending_.size() - done >= 16) {
        out_ << hex_line(offset_, std::string_view(pending_).substr(done, 16));
        offset_ += 16;
        done += 16;
    }
    pending_.erase(0, done);
}

void Dumper::finish() {
    if (!pending_.empty()) {
        out_ << hex_line(offset_, pending_);
        offset_ += pending_.size();
        pending_.clear();
    }
    if (offset_ > 0) {
        out_ << std::format("{:08x}\n", offset_);
    }
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
