// The course's wc from lesson 1.1.7, reading with file descriptors. You
// won't need to change this file.
#include "wc.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>

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

int run_wc(const std::vector<std::string_view>& args, int in_fd, std::ostream& out,
           std::ostream& err) {
    Options o = parse_options(args);
    if (o.bad != 0) {
        err << "wc: invalid option -- '" << o.bad << "'\n";
        return 1;
    }
    int width = column_width(o);
    int status = 0;
    if (o.files.empty()) {
        Counts c;
        if (!count_fd(in_fd, c)) {
            err << "wc: -: " << std::strerror(errno) << '\n';
            status = 1;
        }
        out << format_counts(c, o, width) << '\n';
        return status;
    }
    Counts total;
    for (std::string_view name : o.files) {
        Counts c;
        if (name == "-") {
            if (!count_fd(in_fd, c)) {
                err << "wc: -: " << std::strerror(errno) << '\n';
                status = 1;
            }
        } else {
            int fd = open(std::string(name).c_str(), O_RDONLY);
            if (fd < 0) {
                err << "wc: " << name << ": " << std::strerror(errno) << '\n';
                status = 1;
                continue;
            }
            if (!count_fd(fd, c)) {
                err << "wc: " << name << ": " << std::strerror(errno) << '\n';
                status = 1;
            }
            close(fd);
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
