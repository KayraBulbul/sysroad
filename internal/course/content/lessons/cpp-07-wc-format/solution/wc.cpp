#include "wc.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
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

namespace fs = std::filesystem;

int column_width(const Options& o) {
    int shown = o.lines + o.words + o.bytes;
    std::size_t inputs = std::max<std::size_t>(o.files.size(), 1);
    if (inputs == 1 && shown == 1) {
        return 1;
    }
    int min_width = o.files.empty() ? 7 : 1;
    std::uintmax_t total = 0;
    for (std::string_view name : o.files) {
        if (name == "-") {
            min_width = 7;
            continue;
        }
        fs::path p(name);
        std::error_code ec;
        if (fs::is_regular_file(p, ec)) {
            total += fs::file_size(p, ec);
        } else if (fs::exists(p, ec)) {
            min_width = 7;
        }
    }
    int width = 1;
    while (total >= 10) {
        total /= 10;
        ++width;
    }
    return std::max(width, min_width);
}

std::string format_counts(const Counts& c, const Options& o, int width) {
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
        out += std::format("{:>{}}", cols[i], width);
    }
    return out;
}

int run_wc(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
           std::ostream& err) {
    Options o = parse_options(args);
    if (o.bad != 0) {
        err << "wc: invalid option -- '" << o.bad << "'\n";
        return 1;
    }
    if (o.files.empty()) {
        out << format_counts(count_stream(in), o, column_width(o)) << '\n';
        return 0;
    }
    int width = column_width(o);
    Counts total;
    int status = 0;
    for (std::string_view name : o.files) {
        Counts c;
        if (name == "-") {
            c = count_stream(in);
        } else {
            std::ifstream f{std::string(name), std::ios::binary};
            if (!f) {
                err << "wc: " << name << ": " << std::strerror(errno) << '\n';
                status = 1;
                continue;
            }
            c = count_stream(f);
        }
        out << format_counts(c, o, width) << ' ' << name << '\n';
        total.lines += c.lines;
        total.words += c.words;
        total.bytes += c.bytes;
    }
    if (o.files.size() > 1) {
        out << format_counts(total, o, width) << " total\n";
    }
    return status;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_wc(args, std::cin, std::cout, std::cerr);
}
