// Lists an archive's members and its symbol index, like `ar tv` and
// `nm --print-armap` together. You won't need to change this file.
//
//   artool [FILE.a]     with no file, a small built-in archive
#include <cstdio>
#include <exception>
#include <fstream>
#include <sstream>

#include "archive.hpp"
#include "demo.hpp"

int main(int argc, char** argv) {
    std::string bytes;
    if (argc > 1) {
        std::ifstream in(argv[1], std::ios::binary);
        if (!in) {
            std::fprintf(stderr, "artool: can't open %s\n", argv[1]);
            return 1;
        }
        std::stringstream ss;
        ss << in.rdbuf();
        bytes = ss.str();
    } else {
        bytes.assign(reinterpret_cast<const char*>(demo_archive), sizeof demo_archive);
        std::printf("(the built-in archive; pass a .a file to read your own)\n\n");
    }
    try {
        Archive a = parse_archive(bytes);
        std::printf("%zu members:\n", a.members.size());
        for (const Member& m : a.members) {
            std::printf("  %-36s %8zu bytes\n", m.name.c_str(), m.data.size());
        }
        std::printf("%zu symbols:\n", a.symbols.size());
        for (const auto& [sym, member] : a.symbols) {
            std::printf("  %-36s in %s\n", sym.c_str(), member.c_str());
        }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "artool: %s\n", e.what());
        return 1;
    }
}
