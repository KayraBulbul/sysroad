#include <cctype>

#include "tally.hpp"

int count_words(std::string_view text) {
    int n = 0;
    bool in_word = false;
    for (unsigned char c : text) {
        if (std::isspace(c) || c == '-') {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            ++n;
        }
    }
    return n;
}
