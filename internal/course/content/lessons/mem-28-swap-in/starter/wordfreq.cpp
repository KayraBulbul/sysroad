// wordfreq [-n N] [FILE...]: the N most common words (10 by default) in the
// files, or in standard input.
#include "wordfreq.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <string_view>

void count_words(std::istream& in, HashMap<String, std::size_t>& counts) {
    // TODO: build each word in a String, then count it.
    (void)in;
    (void)counts;
}

Vector<WordCount> top_words(const HashMap<String, std::size_t>& counts, std::size_t n) {
    // TODO: collect with for_each, sort, keep the first n.
    (void)counts;
    (void)n;
    return {};
}

int main(int argc, char** argv) {
    // TODO: options, files, then print each word as uniq -c does: "%7d word".
    (void)argc;
    (void)argv;
    return 0;
}
