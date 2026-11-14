#include "../tally.hpp"

int count_lines(const std::string& text) {
    int n = 0;
    for (char c : text) {
        if (c == '\n') ++n;
    }
    return n;
}

std::string report(std::string_view text) {
    return "words: " + std::to_string(count_words(text)) + "\nlines: " + std::to_string(count_lines(std::string(text))) +
           "\n";
}
