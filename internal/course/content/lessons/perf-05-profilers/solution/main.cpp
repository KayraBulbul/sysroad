// Prints the ten most common words. You won't need to change this file.
//
//   wordfreq FILE...         counts the words in the files
//   wordfreq --demo N        counts N words of made-up text
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

#include "wordfreq.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: wordfreq FILE... | wordfreq --demo N\n");
        return 2;
    }
    std::string text;
    if (std::strcmp(argv[1], "--demo") == 0) {
        std::size_t n = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 100000;
        text = generate_text(n, n / 4 + 1);
    } else {
        for (int i = 1; i < argc; ++i) {
            std::ifstream in(argv[i], std::ios::binary);
            if (!in) {
                std::fprintf(stderr, "wordfreq: can't open %s\n", argv[i]);
                return 1;
            }
            std::stringstream ss;
            ss << in.rdbuf();
            text += ss.str();
            text += '\n';
        }
    }

    auto start = std::chrono::steady_clock::now();
    std::vector<std::string> words = split_words(text);
    std::vector<WordCount> counts = count_words(words);
    std::vector<WordCount> top = top_n(counts, 10);
    std::chrono::duration<double, std::milli> took = std::chrono::steady_clock::now() - start;

    for (const WordCount& wc : top) {
        std::printf("%8ld  %s\n", wc.count, wc.word.c_str());
    }
    std::printf("%zu words, %zu different, in %.0f ms\n", words.size(), counts.size(), took.count());
}
