#include "grep.hpp"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

namespace fs = std::filesystem;

GrepOptions parse_grep_options(const std::vector<std::string_view>& args) {
    GrepOptions o;
    for (std::string_view arg : args) {
        if (!arg.starts_with('-') || arg.size() == 1) {
            if (!o.has_pattern) {
                o.pattern = arg;
                o.has_pattern = true;
            } else {
                o.files.push_back(arg);
            }
            continue;
        }
        for (char ch : arg.substr(1)) {
            switch (ch) {
            case 'i':
                o.ignore_case = true;
                break;
            case 'v':
                o.invert = true;
                break;
            case 'n':
                o.line_numbers = true;
                break;
            case 'c':
                o.count = true;
                break;
            case 'r':
                o.recursive = true;
                break;
            default:
                o.bad = ch;
                return o;
            }
        }
    }
    return o;
}

static bool same_letter(char a, char b) {
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

bool matches(std::string_view line, const GrepOptions& o) {
    bool found;
    if (o.ignore_case) {
        found = std::search(line.begin(), line.end(), o.pattern.begin(), o.pattern.end(),
                            same_letter) != line.end() ||
                o.pattern.empty();
    } else {
        found = line.find(o.pattern) != std::string_view::npos;
    }
    return found != o.invert;
}

std::size_t grep_stream(std::istream& in, std::string_view prefix, const GrepOptions& o,
                        std::ostream& out) {
    std::size_t selected = 0, n = 0;
    std::string line;
    while (std::getline(in, line)) {
        ++n;
        if (!matches(line, o)) {
            continue;
        }
        ++selected;
        if (o.count) {
            continue;
        }
        out << prefix;
        if (o.line_numbers) {
            out << n << ':';
        }
        out << line << '\n';
    }
    if (o.count) {
        out << prefix << selected << '\n';
    }
    return selected;
}

int run_grep(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
             std::ostream& err) {
    GrepOptions o = parse_grep_options(args);
    if (o.bad != 0) {
        err << "grep: invalid option -- '" << o.bad << "'\n";
        return 2;
    }
    if (!o.has_pattern) {
        err << "Usage: grep [OPTION]... PATTERNS [FILE]...\n";
        return 2;
    }
    bool implicit_dot = o.files.empty() && o.recursive;
    std::vector<std::string_view> files = o.files;
    if (files.empty()) {
        files.push_back(o.recursive ? "." : "-");
    }
    bool names = files.size() > 1;
    std::error_code ec;
    for (std::string_view f : files) {
        if (o.recursive && fs::is_directory(fs::path(f), ec)) {
            names = true;
        }
    }

    bool selected = false, error = false;
    auto search_file = [&](const fs::path& path, const std::string& shown) {
        std::ifstream f(path, std::ios::binary);
        if (!f) {
            err << "grep: " << shown << ": " << std::strerror(errno) << '\n';
            error = true;
            return;
        }
        std::string data(std::istreambuf_iterator<char>(f), {});
        bool binary = data.find('\0') != std::string::npos;
        if (binary) {
            std::replace(data.begin(), data.end(), '\0', '\n');  // zero bytes end lines too
        }
        std::istringstream text(data);
        std::string prefix = names ? shown + ":" : "";
        if (!binary || o.count) {
            selected |= grep_stream(text, prefix, o, out) > 0;
            return;
        }
        std::ostringstream discard;
        if (grep_stream(text, prefix, o, discard) > 0) {
            err << "grep: " << shown << ": binary file matches\n";
            selected = true;
        }
    };

    for (std::string_view name : files) {
        if (name == "-") {
            selected |= grep_stream(in, names ? "(standard input):" : "", o, out) > 0;
            continue;
        }
        fs::path p(name);
        if (!fs::is_directory(p, ec)) {
            search_file(p, std::string(name));
            continue;
        }
        if (!o.recursive) {
            err << "grep: " << name << ": Is a directory\n";
            if (o.count) {
                out << (names ? std::string(name) + ":" : "") << "0\n";
            }
            error = true;
            continue;
        }
        std::vector<fs::path> found;
        auto opts = fs::directory_options::skip_permission_denied;
        for (auto it = fs::recursive_directory_iterator(p, opts, ec);
             it != fs::recursive_directory_iterator(); it.increment(ec)) {
            if (!it->is_symlink() && it->is_regular_file()) {
                found.push_back(it->path());
            }
        }
        std::sort(found.begin(), found.end());
        for (const fs::path& f : found) {
            search_file(f, implicit_dot ? f.lexically_relative(".").string() : f.string());
        }
    }
    if (error) {
        return 2;
    }
    return selected ? 0 : 1;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_grep(args, std::cin, std::cout, std::cerr);
}
