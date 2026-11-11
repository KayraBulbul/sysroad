#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

struct WordCount {
    std::string word;
    long count = 0;
    bool operator==(const WordCount&) const = default;
};

// The words in text: runs of the letters a-z and A-Z, lowercased.
// Everything else separates words.
std::vector<std::string> split_words(std::string_view text);

// How many times each distinct word appears, in any order.
std::vector<WordCount> count_words(const std::vector<std::string>& words);

// The n most common words, most common first. Ties go in alphabetical order.
std::vector<WordCount> top_n(std::vector<WordCount> counts, std::size_t n);

// A made-up text of `words` words, drawn from `vocabulary` distinct ones.
// A few words are very common and most are rare, as in real text. The
// same arguments always give the same text.
std::string generate_text(std::size_t words, std::size_t vocabulary);
