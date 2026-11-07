#include "cat.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

CatOptions parse_cat_options(const std::vector<std::string_view>& args) {
    CatOptions o;
    for (std::string_view arg : args) {
        if (!arg.starts_with('-') || arg.size() == 1) {
            o.files.push_back(arg);
            continue;
        }
        for (char ch : arg.substr(1)) {
            switch (ch) {
            case 'n':
                o.number = true;
                break;
            case 'b':
                o.number_nonblank = true;
                break;
            case 's':
                o.squeeze = true;
                break;
            case 'E':
                o.show_ends = true;
                break;
            case 'T':
                o.show_tabs = true;
                break;
            case 'v':
                o.show_nonprinting = true;
                break;
            case 'A':
                o.show_nonprinting = o.show_ends = o.show_tabs = true;
                break;
            case 'e':
                o.show_nonprinting = o.show_ends = true;
                break;
            case 't':
                o.show_nonprinting = o.show_tabs = true;
                break;
            default:
                o.bad = ch;
                return o;
            }
        }
    }
    return o;
}

bool plain(const CatOptions& o) {
    return !(o.number || o.number_nonblank || o.squeeze || o.show_ends || o.show_tabs ||
             o.show_nonprinting);
}

std::size_t copy_stream(std::istream& in, std::ostream& out) {
    std::array<char, 65536> buf;
    std::size_t total = 0;
    while (in.read(buf.data(), buf.size()) || in.gcount() > 0) {
        out.write(buf.data(), in.gcount());
        total += static_cast<std::size_t>(in.gcount());
    }
    return total;
}

void cat_stream(std::istream& in, std::ostream& out, const CatOptions& o, CatState& st) {
    std::array<char, 4096> buf;
    std::string text;
    while (in.read(buf.data(), buf.size()) || in.gcount() > 0) {
        text.clear();
        for (char ch : std::string_view(buf.data(), static_cast<std::size_t>(in.gcount()))) {
            if (st.at_line_start) {
                bool numbered = o.number_nonblank ? ch != '\n' : o.number;
                if (numbered) {
                    text += std::format("{:>6}\t", ++st.line);
                }
            }
            st.at_line_start = ch == '\n';
            text += ch;
        }
        out << text;
    }
}

int run_cat(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
            std::ostream& err) {
    CatOptions o = parse_cat_options(args);
    if (o.bad != 0) {
        err << "cat: invalid option -- '" << o.bad << "'\n";
        return 1;
    }
    if (o.files.empty()) {
        o.files.push_back("-");
    }
    CatState st;
    int status = 0;
    auto copy = [&](std::istream& from) {
        if (plain(o)) {
            copy_stream(from, out);
        } else {
            cat_stream(from, out, o, st);
        }
    };
    for (std::string_view name : o.files) {
        if (name == "-") {
            copy(in);
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
        copy(f);
    }
    if (st.pending_cr) {
        out << '\r';  // the input ended right after it
    }
    return status;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_cat(args, std::cin, std::cout, std::cerr);
}
