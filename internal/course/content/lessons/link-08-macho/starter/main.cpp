// otool -h -l -L and nm, in one, for 64-bit Mach-O files. You won't need to
// change this file.
//
//   machotool FILE         any Mach-O file, like /bin/ls on the Mac. A
//                          universal binary is cut down to one CPU's part.
//   machotool macho-exe    a built-in sample: macho-exe (the default) or macho.o
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "macho.hpp"
#include "samples.hpp"

static std::uint32_t read_be32(std::string_view b, std::size_t at) {
    if (at + 4 > b.size()) {
        throw std::out_of_range("universal header cut short");
    }
    std::uint32_t v = 0;
    for (int i = 0; i < 4; ++i) {
        v = v << 8 | static_cast<unsigned char>(b[at + static_cast<std::size_t>(i)]);
    }
    return v;
}

// A universal ("fat") binary is a big-endian table of (cputype, subtype,
// offset, size, align) followed by one ordinary Mach-O file per CPU. This
// returns the arm64 one if there is one, else the first.
static std::string thin(const std::string& b) {
    if (read_be32(b, 0) != 0xcafebabe) {
        return b;
    }
    std::uint32_t n = read_be32(b, 4);
    std::size_t pick = 0;
    for (std::uint32_t i = 0; i < n; ++i) {
        if (read_be32(b, 8 + 20 * i) == 0x0100000c) {
            pick = i;
            break;
        }
    }
    std::size_t at = 8 + 20 * pick;
    std::printf("(a universal binary of %u; reading part %zu)\n\n", n, pick);
    return b.substr(read_be32(b, at + 8), read_be32(b, at + 12));
}

static std::string load(const char* name) {
    if (std::strcmp(name, "macho-exe") == 0) {
        return sample_macho_exe();
    }
    if (std::strcmp(name, "macho.o") == 0) {
        return std::string(bytes_of(sample_macho_o));
    }
    std::ifstream in(name, std::ios::binary);
    if (!in) {
        throw std::runtime_error(std::string("can't open ") + name);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return thin(ss.str());
}

static const char* cpu(std::uint32_t t) {
    return t == 0x0100000c ? "ARM64" : t == 0x01000007 ? "X86_64" : "other";
}

static const char* filetype(std::uint32_t t) {
    switch (t) {
    case 1:
        return "OBJECT";
    case 2:
        return "EXECUTE";
    case 6:
        return "DYLIB";
    case 8:
        return "BUNDLE";
    }
    return "other";
}

// nm's letter: T text, D data, B bss, S other section, U undefined,
// A absolute; lower case for symbols that aren't external.
static char letter(const MachSymbol& s, const std::map<int, std::string>& sect_names) {
    char c = '?';
    switch (s.type & 0x0e) {
    case 0x0:
        c = 'U';
        break;
    case 0x2:
        c = 'A';
        break;
    case 0xe: {
        auto it = sect_names.find(s.sect);
        std::string n = it == sect_names.end() ? "" : it->second;
        c = n == "__TEXT,__text"   ? 'T'
            : n == "__DATA,__data" ? 'D'
            : n == "__DATA,__bss"  ? 'B'
                                   : 'S';
        break;
    }
    }
    return (s.type & 0x01) || c == 'U' ? c : static_cast<char>(c - 'A' + 'a');
}

int main(int argc, char** argv) {
    try {
        std::string b = load(argc > 1 ? argv[1] : "macho-exe");
        MachHeader h = parse_mach_header(b);
        std::printf("Mach header: %s %s, %u load commands, %u bytes of them\n", cpu(h.cputype),
                    filetype(h.filetype), h.ncmds, h.sizeofcmds);

        std::vector<LoadCommand> cmds = load_commands(b, h);
        std::printf("\nLoad commands:\n");
        for (std::size_t i = 0; i < cmds.size(); ++i) {
            std::printf("  %2zu %-28s %5u bytes at %zu\n", i, command_name(cmds[i].cmd).c_str(),
                        cmds[i].size, cmds[i].offset);
        }

        std::printf("\nSegments and sections:\n");
        std::map<int, std::string> sect_names;
        int n = 0;
        for (const MachSegment& seg : segments(b, cmds)) {
            std::printf("  %-16s vmaddr 0x%09llx vmsize 0x%09llx\n",
                        seg.name.empty() ? "(unnamed)" : seg.name.c_str(),
                        (unsigned long long)seg.vmaddr, (unsigned long long)seg.vmsize);
            for (const MachSection& sec : seg.sections) {
                sect_names[++n] = sec.segment + "," + sec.name;
                std::printf("    %2d %-28s addr 0x%09llx size 0x%llx\n", n, sect_names[n].c_str(),
                            (unsigned long long)sec.addr, (unsigned long long)sec.size);
            }
        }

        std::printf("\nLibraries:\n");
        for (const std::string& lib : dylibs(b, cmds)) {
            std::printf("  %s\n", lib.c_str());
        }

        std::printf("\nSymbols:\n");
        for (const MachSymbol& s : symbols(b, cmds)) {
            if (s.type & 0xe0) {
                continue;  // debugging entries ("stabs")
            }
            if ((s.type & 0x0e) == 0) {
                std::printf("  %16s %c %s\n", "", letter(s, sect_names), s.name.c_str());
            } else {
                std::printf("  %016llx %c %s\n", (unsigned long long)s.value, letter(s, sect_names),
                            s.name.c_str());
            }
        }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "machotool: %s\n", e.what());
        return 1;
    }
}
