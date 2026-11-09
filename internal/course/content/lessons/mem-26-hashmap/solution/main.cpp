// hashmap WORD...: count the words on standard input, then look up each WORD.
// You won't need to change this file.
#include <iostream>
#include <string>

#include "hashmap.hpp"

int main(int argc, char** argv) {
    HashMap<std::string, int> counts;
    std::string word;
    while (std::cin >> word) {
        ++counts[word];
    }
    for (int i = 1; i < argc; ++i) {
        const int* n = counts.find(argv[i]);
        std::cout << argv[i] << ": " << (n != nullptr ? *n : 0) << '\n';
    }
    std::cout << counts.size() << " different words, capacity " << counts.capacity() << '\n';
}
