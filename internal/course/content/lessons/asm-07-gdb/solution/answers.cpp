// Prints the command that tries your answers on the vault.
// You won't need to change this file.
#include <iostream>

#include "answers.hpp"

int main() {
    std::cout << "Try your answers in the lesson folder with:\n\n";
    std::cout << "  ./vault " << (kWord.empty() ? "WORD" : kWord) << ' ' << kNumber << ' ' << kIndex
              << '\n';
}
