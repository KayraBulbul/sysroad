// echo [-n] [ARG...]: print the arguments, separated by spaces. -n leaves
// out the newline. You won't need to change this file.
#include <iostream>
#include <string_view>

#include "coreutils/io.hpp"

int main(int argc, char** argv) {
    auto words = coreutils::args(argc, argv);
    bool newline = true;
    if (!words.empty() && std::string_view(words[0]) == "-n") {
        newline = false;
        words = words.subspan(1);
    }
    for (std::size_t i = 0; i < words.size(); ++i) {
        std::cout << (i > 0 ? " " : "") << words[i];
    }
    if (newline) {
        std::cout << '\n';
    }
}
