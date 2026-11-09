// wordfreq [-n N] [FILE...]: the N most common words (10 by default) in the
// files, or in standard input.
#include "wordfreq.hpp"

#include <algorithm>
#include <charconv>
#include <format>
#include <fstream>
#include <iostream>
#include <string_view>

static bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void count_words(std::istream& in, HashMap<String, std::size_t>& counts) {
    String word;
    char c;
    while (in.get(c)) {
        if (is_letter(c)) {
            word.push_back(static_cast<char>(c | 0x20));  // ASCII lowercase
        } else if (!word.empty()) {
            ++counts[word];
            word.clear();
        }
    }
    if (!word.empty()) {
        ++counts[word];
    }
}

Vector<WordCount> top_words(const HashMap<String, std::size_t>& counts, std::size_t n) {
    Vector<WordCount> all;
    all.reserve(counts.size());
    counts.for_each(
        [&](const String& w, std::size_t count) { all.push_back(WordCount{w, count}); });
    std::sort(all.begin(), all.end(), [](const WordCount& a, const WordCount& b) {
        if (a.count != b.count) {
            return a.count > b.count;
        }
        return a.word < b.word;
    });
    if (all.size() > n) {
        all.erase(all.begin() + n, all.end());
    }
    return all;
}

int main(int argc, char** argv) {
    std::size_t n = 10;
    int first = 1;
    if (argc > 1 && std::string_view(argv[1]) == "-n") {
        std::string_view num = argc > 2 ? argv[2] : "";
        auto [end, ec] = std::from_chars(num.data(), num.data() + num.size(), n);
        if (num.empty() || ec != std::errc() || end != num.data() + num.size()) {
            std::cerr << "usage: wordfreq [-n N] [FILE...]\n";
            return 2;
        }
        first = 3;
    }
    HashMap<String, std::size_t> counts;
    int status = 0;
    if (first >= argc) {
        count_words(std::cin, counts);
    }
    for (int i = first; i < argc; ++i) {
        std::ifstream in(argv[i], std::ios::binary);
        if (!in) {
            std::cerr << "wordfreq: " << argv[i] << ": can't open\n";
            status = 1;
            continue;
        }
        count_words(in, counts);
    }
    for (const WordCount& wc : top_words(counts, n)) {
        std::cout << std::format("{:7} {}\n", wc.count, std::string_view(wc.word));
    }
    return status;
}
