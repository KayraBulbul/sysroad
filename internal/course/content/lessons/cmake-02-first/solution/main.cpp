// hello NAME...: greet each name. You won't need to change this file.
#include <iostream>

#include "greet.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << greet("") << '\n';
    }
    for (int i = 1; i < argc; ++i) {
        std::cout << greet(argv[i]) << '\n';
    }
}
