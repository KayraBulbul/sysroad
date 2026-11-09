// bmp FILE [X Y]: describe a BMP image, or print one pixel's colour.
#include <charconv>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "bmp.hpp"

static bool parse_int(const char* s, int& out) {
    std::string_view sv(s);
    auto [end, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);
    return ec == std::errc() && end == sv.data() + sv.size();
}

int main(int argc, char** argv) {
    int x = 0;
    int y = 0;
    if ((argc != 2 && argc != 4) ||
        (argc == 4 && (!parse_int(argv[2], x) || !parse_int(argv[3], y)))) {
        std::cerr << "usage: bmp FILE [X Y]\n";
        return 2;
    }
    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
        std::cerr << "bmp: " << argv[1] << ": can't open\n";
        return 1;
    }
    std::vector<unsigned char> file{std::istreambuf_iterator<char>(in),
                                    std::istreambuf_iterator<char>()};
    try {
        BmpInfo info = parse_bmp(file);
        if (argc == 4) {
            Rgb p = pixel_at(file, info, x, y);
            std::cout << std::format("#{:02x}{:02x}{:02x}\n", p.r, p.g, p.b);
        } else {
            std::cout << std::format("{}: {} x {}, {}-bit, {}, {} bytes per row ({} of padding)\n",
                                     argv[1], info.width, info.height, info.bits,
                                     info.top_down ? "top-down" : "bottom-up", info.row_size,
                                     info.row_size - info.width * (info.bits / 8));
        }
    } catch (const BmpError& e) {
        std::cerr << "bmp: " << argv[1] << ": " << e.what() << '\n';
        return 1;
    }
}
