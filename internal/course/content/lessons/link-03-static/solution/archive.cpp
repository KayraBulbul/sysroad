#include "archive.hpp"

#include <cstdint>
#include <map>
#include <stdexcept>

namespace {

constexpr std::string_view magic = "!<arch>\n";
constexpr std::size_t header_size = 60;

[[noreturn]] void bad(const std::string& why) {
    throw std::runtime_error("not a valid archive: " + why);
}

// A header field: ASCII, padded on the right with spaces.
std::string_view field(std::string_view header, std::size_t at, std::size_t len) {
    std::string_view f = header.substr(at, len);
    while (!f.empty() && f.back() == ' ') {
        f.remove_suffix(1);
    }
    return f;
}

std::size_t decimal(std::string_view s) {
    if (s.empty()) {
        bad("empty number");
    }
    std::size_t n = 0;
    for (char c : s) {
        if (c < '0' || c > '9') {
            bad("bad number '" + std::string(s) + "'");
        }
        n = n * 10 + static_cast<std::size_t>(c - '0');
    }
    return n;
}

std::uint32_t read_u32(std::string_view b, std::size_t at, bool big_endian) {
    if (at + 4 > b.size()) {
        bad("symbol table cut short");
    }
    std::uint32_t v = 0;
    for (int i = 0; i < 4; ++i) {
        auto byte = static_cast<unsigned char>(b[at + static_cast<std::size_t>(i)]);
        v |= static_cast<std::uint32_t>(byte) << (big_endian ? 8 * (3 - i) : 8 * i);
    }
    return v;
}

// The NUL-terminated string at `at` in table.
std::string c_string(std::string_view table, std::size_t at) {
    if (at >= table.size()) {
        bad("symbol name out of range");
    }
    std::size_t end = table.find('\0', at);
    return std::string(table.substr(at, end == std::string_view::npos ? end : end - at));
}

}  // namespace

Archive parse_archive(std::string_view bytes) {
    if (!bytes.starts_with(magic)) {
        bad("no !<arch> at the start");
    }
    Archive a;
    std::string_view long_names;                 // GNU's "//" member
    std::string_view gnu_symbols;                // GNU's "/" member
    std::string_view bsd_symbols;                // BSD's "__.SYMDEF" member
    std::map<std::size_t, std::string> name_at;  // header offset -> member name

    std::size_t pos = magic.size();
    while (pos < bytes.size()) {
        if (pos + header_size > bytes.size()) {
            bad("header cut short");
        }
        std::string_view h = bytes.substr(pos, header_size);
        if (h.substr(58, 2) != "`\n") {
            bad("header doesn't end in `\\n");
        }
        std::string_view raw_name = field(h, 0, 16);
        std::size_t size = decimal(field(h, 48, 10));
        if (pos + header_size + size > bytes.size()) {
            bad("member cut short");
        }
        std::string_view data = bytes.substr(pos + header_size, size);

        std::string name;
        if (raw_name.starts_with("#1/")) {
            // BSD: the name is the first n bytes of the data, padded with NULs.
            std::size_t n = decimal(raw_name.substr(3));
            if (n > data.size()) {
                bad("long name cut short");
            }
            std::string_view stored = data.substr(0, n);
            name = std::string(stored.substr(0, stored.find('\0')));
            data.remove_prefix(n);
        } else if (raw_name == "/") {
            gnu_symbols = data;
        } else if (raw_name == "/SYM64/") {
            // The same table with 64-bit numbers, for archives over 4 GB. Skipped.
        } else if (raw_name == "//") {
            long_names = data;
        } else if (raw_name.starts_with('/')) {
            // GNU: /n is the name at offset n in the long-name table, ending "/\n".
            std::size_t at = decimal(raw_name.substr(1));
            if (at >= long_names.size()) {
                bad("long name out of range");
            }
            std::string_view rest = long_names.substr(at);
            name = std::string(rest.substr(0, rest.find("/\n")));
        } else {
            name = std::string(raw_name);
            if (name.ends_with('/')) {
                name.pop_back();  // GNU ends short names with '/'
            }
        }

        if (name.starts_with("__.SYMDEF")) {
            bsd_symbols = data;
        } else if (!name.empty()) {
            a.members.push_back({name, pos, data});
            name_at[pos] = name;
        }
        pos += header_size + size;
        if (pos % 2 == 1) {
            // Members start at even offsets, so an odd one is followed by '\n'.
            if (pos == bytes.size()) {
                bad("the last member's padding byte is missing");
            }
            ++pos;
        }
    }

    auto member_name = [&](std::size_t offset) {
        auto it = name_at.find(offset);
        if (it == name_at.end()) {
            bad("symbol points at no member");
        }
        return it->second;
    };

    if (!gnu_symbols.empty()) {
        // Big-endian count, count offsets, then count NUL-terminated names.
        std::uint32_t count = read_u32(gnu_symbols, 0, true);
        std::size_t names = 4 + 4 * static_cast<std::size_t>(count);
        if (names > gnu_symbols.size()) {
            bad("symbol table cut short");
        }
        for (std::uint32_t i = 0; i < count; ++i) {
            std::uint32_t offset = read_u32(gnu_symbols, 4 + 4 * i, true);
            std::string sym = c_string(gnu_symbols, names);
            names += sym.size() + 1;
            a.symbols.emplace_back(sym, member_name(offset));
        }
    } else if (!bsd_symbols.empty()) {
        // Little-endian: the size in bytes of the (name, offset) pairs, the
        // pairs, then the size of the string table and the strings.
        std::uint32_t pairs = read_u32(bsd_symbols, 0, false);
        std::size_t strings = 4 + pairs + 4;
        if (strings > bsd_symbols.size()) {
            bad("symbol table cut short");
        }
        std::string_view table = bsd_symbols.substr(strings);
        for (std::uint32_t i = 0; i < pairs / 8; ++i) {
            std::uint32_t strx = read_u32(bsd_symbols, 4 + 8 * i, false);
            std::uint32_t offset = read_u32(bsd_symbols, 4 + 8 * i + 4, false);
            a.symbols.emplace_back(c_string(table, strx), member_name(offset));
        }
    }
    return a;
}
