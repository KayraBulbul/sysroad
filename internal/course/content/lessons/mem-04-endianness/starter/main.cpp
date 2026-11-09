// endian FILE...: what each PNG's header says. You won't need to change this file.
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "png.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: endian FILE...\n";
        return 2;
    }
    int status = 0;
    for (int i = 1; i < argc; ++i) {
        std::ifstream in(argv[i], std::ios::binary);
        if (!in) {
            std::cerr << "endian: " << argv[i] << ": can't open\n";
            status = 1;
            continue;
        }
        std::string bytes{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
        if (auto info = parse_png(bytes)) {
            std::cout << argv[i] << ": " << describe(*info) << '\n';
        } else {
            std::cout << argv[i] << ": not a PNG\n";
            status = 1;
        }
    }
    return status;
}
