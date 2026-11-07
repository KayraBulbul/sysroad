#include "hexdump.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <charconv>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>

bool parse_number(std::string_view s, std::uint64_t& out) {
    int base = 10;
    if (s.starts_with("0x") || s.starts_with("0X")) {
        s.remove_prefix(2);
        base = 16;
    }
    if (s.empty()) {
        return false;
    }
    auto [end, ec] = std::from_chars(s.data(), s.data() + s.size(), out, base);
    return ec == std::errc() && end == s.data() + s.size();
}

HexOptions parse_hex_options(const std::vector<std::string_view>& args) {
    HexOptions o;
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view arg = args[i];
        if (!arg.starts_with('-') || arg.size() == 1) {
            o.files.push_back(arg);
            continue;
        }
        for (std::size_t j = 1; j < arg.size(); ++j) {
            char ch = arg[j];
            if (ch == 'C') {
                continue;  // canonical output is the only kind there is
            }
            if (ch == 'v') {
                o.no_squeeze = true;
                continue;
            }
            if (ch != 's' && ch != 'n') {
                o.error = std::format("invalid option -- '{}'", ch);
                return o;
            }
            // The number is the rest of this argument, or the next one.
            std::string_view num = arg.substr(j + 1);
            if (num.empty()) {
                if (i + 1 == args.size()) {
                    o.error = std::format("option requires an argument -- '{}'", ch);
                    return o;
                }
                num = args[++i];
            }
            std::uint64_t n = 0;
            if (!parse_number(num, n)) {
                o.error = std::format("invalid number '{}'", num);
                return o;
            }
            if (ch == 's') {
                o.skip = n;
            } else {
                o.has_length = true;
                o.length = n;
            }
            break;
        }
    }
    return o;
}

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

Dumper::Dumper(std::ostream& out, const HexOptions& o) : out_(out), o_(o), to_skip_(o.skip) {}

void Dumper::feed(std::string_view bytes) {
    // TODO: -s: skip the first o_.skip bytes (count down to_skip_), moving offset_ on
    // TODO: -n: dump at most o_.length bytes (count o_.length down)
    if (bytes.empty()) {
        return;
    }
    consumed_ = true;
    pending_ += bytes;
    std::size_t done = 0;
    while (pending_.size() - done >= 16) {
        line(std::string_view(pending_).substr(done, 16));
        done += 16;
    }
    pending_.erase(0, done);
}

void Dumper::line(std::string_view bytes) {
    // TODO: squeeze a full line that's the same as the one before into "*" (unless -v)
    out_ << hex_line(offset_, bytes);
    offset_ += bytes.size();
}

void Dumper::finish() {
    if (!pending_.empty()) {
        line(pending_);
        pending_.clear();
    }
    if (consumed_) {
        out_ << std::format("{:08x}\n", offset_);
    }
}

int run_hexdump(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
                std::ostream& err) {
    HexOptions o = parse_hex_options(args);
    if (!o.error.empty()) {
        err << "hexdump: " << o.error << '\n';
        return 1;
    }
    if (o.has_length && o.length == 0) {
        return 0;  // -n 0: nothing at all, not even an offset
    }
    if (o.files.empty()) {
        o.files.push_back("-");
    }
    Dumper d(out, o);
    std::array<char, 4096> buf;
    auto feed_all = [&](std::istream& from) {
        while (from.read(buf.data(), buf.size()) || from.gcount() > 0) {
            d.feed(std::string_view(buf.data(), static_cast<std::size_t>(from.gcount())));
        }
    };
    int status = 0;
    for (std::string_view name : o.files) {
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
