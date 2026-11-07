#include "wc.hpp"

#include <array>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

Counts count_stream(std::istream& in) {
    Counts c;
    std::array<char, 4096> buf;
    bool in_word = false;
    while (in.read(buf.data(), buf.size()) || in.gcount() > 0) {
        std::string_view chunk(buf.data(), static_cast<std::size_t>(in.gcount()));
        c.bytes += chunk.size();
        for (char ch : chunk) {
            if (ch == '\n') {
                ++c.lines;
            }
            bool space = std::isspace(static_cast<unsigned char>(ch));
            if (!space && !in_word) {
                ++c.words;
            }
            in_word = !space;
        }
    }
    return c;
}

Options parse_options(const std::vector<std::string_view>& args) {
    Options o;
    for (std::string_view arg : args) {
        if (!arg.starts_with('-') || arg.size() == 1) {
            o.files.push_back(arg);
            continue;
        }
        for (char ch : arg.substr(1)) {
            switch (ch) {
            case 'l':
                o.lines = true;
                break;
            case 'w':
                o.words = true;
                break;
            case 'c':
                o.bytes = true;
                break;
            default:
                o.bad = ch;
                return o;
            }
        }
    }
    if (!o.lines && !o.words && !o.bytes) {
        o.lines = o.words = o.bytes = true;
    }
    return o;
}

std::string format_counts(const Counts& c, const Options& o) {
    std::vector<std::size_t> cols;
    if (o.lines) {
        cols.push_back(c.lines);
    }
    if (o.words) {
        cols.push_back(c.words);
    }
    if (o.bytes) {
        cols.push_back(c.bytes);
    }
    std::string out;
    for (std::size_t i = 0; i < cols.size(); ++i) {
        if (i > 0) {
            out += ' ';
        }
        out += std::format("{:>7}", cols[i]);
    }
    return out;
}

int run_wc(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
           std::ostream& err) {
    // TODO: flags, then stdin or each file, errors, and the total
    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_wc(args, std::cin, std::cout, std::cerr);
}
