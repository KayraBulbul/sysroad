#include "histogram.hpp"

#include <format>
#include <iostream>
#include <iterator>
#include <string>

void Histogram::add(std::string_view bytes) {
    // TODO
}

std::size_t Histogram::count(unsigned char b) const {
    return 0;  // TODO
}

std::size_t Histogram::total() const {
    return 0;  // TODO
}

unsigned char Histogram::most_common() const {
    return 0;  // TODO
}

int main() {
    // All of stdin as one string.
    std::string input(std::istreambuf_iterator<char>(std::cin), {});
    Histogram h;
    // TODO: add the input and print the summary line
    return 0;
}
