// You won't need to change this file.
#include "textutil/textutil.hpp"

namespace textutil {

namespace {
bool is_space(char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
}
}  // namespace

Counts count(std::string_view text) {
    Counts c;
    c.bytes = text.size();
    bool in_word = false;
    for (char ch : text) {
        if (ch == '\n') {
            ++c.lines;
        }
        if (is_space(ch)) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            ++c.words;
        }
    }
    return c;
}

Counts total(std::span<const Counts> parts) {
    Counts sum;
    for (const Counts& c : parts) {
        sum.lines += c.lines;
        sum.words += c.words;
        sum.bytes += c.bytes;
    }
    return sum;
}

std::vector<std::string_view> split_words(std::string_view text) {
    std::vector<std::string_view> words;
    std::size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && is_space(text[i])) {
            ++i;
        }
        std::size_t start = i;
        while (i < text.size() && !is_space(text[i])) {
            ++i;
        }
        if (i > start) {
            words.push_back(text.substr(start, i - start));
        }
    }
    return words;
}

std::string_view trim(std::string_view text) {
    while (!text.empty() && is_space(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && is_space(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

}  // namespace textutil
