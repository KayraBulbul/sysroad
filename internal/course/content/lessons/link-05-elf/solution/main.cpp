// Says what kind of file each argument is, like file(1). You won't need to
// change this file.
//
//   ident FILE...
#include <cstdio>
#include <fstream>
#include <sstream>

#include "ident.hpp"
#include "samples.hpp"

int main(int argc, char** argv) {
    std::printf("%-22s %s\n", "(sample x86-64 .o)", identify(bytes_of(sample_x86_64_o)).c_str());
    std::printf("%-22s %s\n", "(sample Mach-O .o)", identify(bytes_of(sample_macho_o)).c_str());
    // The first argument is always this program itself.
    for (int i = 0; i < argc; ++i) {
        std::ifstream in(argv[i], std::ios::binary);
        if (!in) {
            std::printf("%-22s can't open\n", argv[i]);
            continue;
        }
        std::stringstream ss;
        ss << in.rdbuf();
        std::printf("%-22s %s\n", i == 0 ? "(this program)" : argv[i], identify(ss.str()).c_str());
    }
}
