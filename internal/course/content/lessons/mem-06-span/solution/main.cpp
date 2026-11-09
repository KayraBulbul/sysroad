// reader FILE: list the players in a scores file. You won't need to change this file.
//
// The format, all numbers little-endian:
//   "SCOR"  u16 version  u16 count
//   then count times:  u8 name length  name bytes  u32 score
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "reader.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: reader FILE\n";
        return 2;
    }
    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
        std::cerr << "reader: " << argv[1] << ": can't open\n";
        return 1;
    }
    std::vector<unsigned char> file{std::istreambuf_iterator<char>(in),
                                    std::istreambuf_iterator<char>()};
    ByteReader r(file);

    auto magic = r.bytes(4);
    if (!r.ok() || std::string(magic.begin(), magic.end()) != "SCOR") {
        std::cerr << "reader: " << argv[1] << ": not a scores file\n";
        return 1;
    }
    unsigned version = r.le16();
    unsigned count = r.le16();
    if (!r.ok()) {
        std::cerr << "reader: " << argv[1] << ": cut off in the header\n";
        return 1;
    }
    std::printf("version %u, %u %s\n", version, count, count == 1 ? "player" : "players");
    for (unsigned i = 0; i < count; ++i) {
        auto name = r.bytes(r.u8());
        std::uint32_t score = r.le32();
        if (!r.ok()) {
            std::cerr << "reader: " << argv[1] << ": cut off after " << i << " of " << count
                      << " players\n";
            return 1;
        }
        std::printf("%-10s %6u\n", std::string(name.begin(), name.end()).c_str(), score);
    }
    if (r.remaining() > 0) {
        std::printf("(%zu bytes left over)\n", r.remaining());
    }
}
