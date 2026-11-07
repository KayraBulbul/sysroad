#include "histogram.hpp"

#include <format>
#include <iostream>
#include <iterator>
#include <string>

void Histogram::add(std::string_view bytes) {
    for (char ch : bytes) {
        ++counts[static_cast<unsigned char>(ch)];
    }
}

std::size_t Histogram::count(unsigned char b) const {
    return counts[b];
}

std::size_t Histogram::total() const {
    std::size_t sum = 0;
    for (std::size_t n : counts) {
        sum += n;
    }
    return sum;
}

unsigned char Histogram::most_common() const {
    int best = 0;
    for (int b = 1; b < 256; ++b) {
        if (counts[b] > counts[best]) {
            best = b;
        }
    }
    return static_cast<unsigned char>(best);
}

int main() {
    // All of stdin as one string.
    std::string input(std::istreambuf_iterator<char>(std::cin), {});
    Histogram h;
    h.add(input);
    unsigned char b = h.most_common();
    std::cout << std::format("most common: 0x{:02x} ({} of {})\n", static_cast<int>(b), h.count(b),
                             h.total());
    return 0;
}
