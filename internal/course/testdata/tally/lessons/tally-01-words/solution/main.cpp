#include <iostream>
#include <iterator>
#include <string>

#include "tally.hpp"

int main() {
    std::string text{std::istreambuf_iterator<char>(std::cin), {}};
    std::cout << count_words(text) << '\n';
}
