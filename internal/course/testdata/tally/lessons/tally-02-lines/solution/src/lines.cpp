#include "../tally.hpp"

int count_lines(const std::string& text) {
    int n = 0;
    for (char c : text) {
        if (c == '\n') ++n;
    }
    return n;
}

Stats stats(const std::string& text) { return {count_words(text), count_lines(text)}; }
