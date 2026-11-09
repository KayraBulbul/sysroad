#include "bugs.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstring>
#include <sstream>

int sum_numbers(const std::string& text) {
    int* values = new int[text.size() + 1];
    std::size_t n = 0;
    std::istringstream in(text);
    std::string word;
    while (in >> word) {
        auto [end, ec] = std::from_chars(word.data(), word.data() + word.size(), values[n]);
        if (ec != std::errc() || end != word.data() + word.size()) {
            delete[] values;
            return -1;
        }
        ++n;
    }
    int total = 0;
    for (std::size_t i = 0; i < n; ++i) {
        total += values[i];
    }
    delete[] values;
    return total;
}

std::string shout(const std::string& text) {
    std::size_t len = std::min(text.find('\n'), text.size());
    char* line = new char[len + 1];
    std::memcpy(line, text.data(), len);
    line[len] = '\0';
    for (char* p = line; *p != '\0'; ++p) {
        *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
    }
    std::string result = line;
    delete[] line;
    return result;
}

int* doubled_copy(const int* src, std::size_t n) {
    if (n == 0) {
        return nullptr;
    }
    int* out = new int[n];
    for (std::size_t i = 0; i < n; ++i) {
        out[i] = 2 * src[i];
    }
    return out;
}

std::size_t count_unique(const std::string& text) {
    std::string* seen = new std::string[text.size() / 2 + 1];
    std::size_t n = 0;
    std::istringstream in(text);
    std::string word;
    while (in >> word) {
        if (std::find(seen, seen + n, word) == seen + n) {
            seen[n++] = word;
        }
    }
    delete[] seen;
    return n;
}
