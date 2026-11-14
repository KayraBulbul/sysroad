#include <cctype>

#include "tally.hpp"

int count_words(const std::string& text) {
    int n = 0;
    bool in_word = false;
    for (unsigned char c : text) {
        if (std::isspace(c)) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            ++n;
        }
    }
    return n;
}
