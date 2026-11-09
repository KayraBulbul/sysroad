#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <istream>

#include "hashmap.hpp"
#include "string.hpp"
#include "vector.hpp"

struct WordCount {
    String word;
    std::size_t count = 0;
};

// Adds the words read from in to counts. A word is a run of ASCII letters,
// lowercased; everything else separates words.
void count_words(std::istream& in, HashMap<String, std::size_t>& counts);

// The n most common words: the highest count first, and words with equal
// counts in byte order ("apple" before "banana").
Vector<WordCount> top_words(const HashMap<String, std::size_t>& counts, std::size_t n);
