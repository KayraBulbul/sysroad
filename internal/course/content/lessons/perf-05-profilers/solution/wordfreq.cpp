#include "wordfreq.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <unordered_map>

std::vector<std::string> split_words(std::string_view text) {
    std::vector<std::string> words;
    std::string word;
    for (char c : text) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            word = word + static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (!word.empty()) {
            words.push_back(word);
            word = "";
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

std::vector<WordCount> count_words(const std::vector<std::string>& words) {
    std::unordered_map<std::string_view, long> seen;
    seen.reserve(words.size() / 4);
    for (const std::string& w : words) {
        ++seen[w];
    }
    std::vector<WordCount> counts;
    counts.reserve(seen.size());
    for (const auto& [word, count] : seen) {
        counts.push_back({std::string(word), count});
    }
    return counts;
}

std::vector<WordCount> top_n(std::vector<WordCount> counts, std::size_t n) {
    std::sort(counts.begin(), counts.end(), [](const WordCount& a, const WordCount& b) {
        if (a.count != b.count) {
            return a.count > b.count;
        }
        return a.word < b.word;
    });
    if (counts.size() > n) {
        counts.resize(n);
    }
    return counts;
}

// ---- The rest makes up test input. You won't need to change it. ----

// The i-th made-up word: syllables picked by the digits of i in base 30.
static std::string made_up_word(std::size_t i) {
    static const char* syllables[] = {"ka", "ro", "mi", "tu", "ne", "sa", "lo", "pi", "da", "ve",
                                      "zu", "ha", "fo", "ri", "ba", "ce", "no", "gu", "le", "ti",
                                      "mo", "wa", "su", "pe", "ju", "xa", "bo", "di", "ya", "qu"};
    std::string w;
    do {
        w += syllables[i % 30];
        i /= 30;
    } while (i > 0);
    return w;
}

std::string generate_text(std::size_t words, std::size_t vocabulary) {
    // Word r turns up in proportion to 1 / (r + 1): Zipf's law.
    std::vector<double> cumulative(vocabulary);
    double total = 0;
    for (std::size_t r = 0; r < vocabulary; ++r) {
        total += 1.0 / static_cast<double>(r + 1);
        cumulative[r] = total;
    }
    std::uint64_t state = 0x9e3779b97f4a7c15;  // xorshift64: the same numbers everywhere
    std::string text;
    for (std::size_t n = 0; n < words; ++n) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        double u = static_cast<double>(state >> 11) / 9007199254740992.0 * total;
        std::size_t r =
            std::lower_bound(cumulative.begin(), cumulative.end(), u) - cumulative.begin();
        text += made_up_word(std::min(r, vocabulary - 1));
        text += n % 12 == 11 ? ".\n" : " ";
    }
    return text;
}
