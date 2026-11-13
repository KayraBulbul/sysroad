#include <string>

#include "shell.hpp"

std::vector<std::string> split_words(const std::string& line) {
    std::vector<std::string> words;
    std::string word;
    bool in_word = false;
    for (char c : line) {
        if (c == ' ' || c == '\t' || c == '\n') {
            if (in_word) {
                words.push_back(word);
                word.clear();
                in_word = false;
            }
            continue;
        }
        if (c == '#' && !in_word) {
            break;  // a comment, to the end of the line
        }
        word += c;
        in_word = true;
    }
    if (in_word) {
        words.push_back(word);
    }
    return words;
}
