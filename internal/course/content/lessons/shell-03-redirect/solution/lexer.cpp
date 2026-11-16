#include <cctype>

#include "conch.hpp"

// Characters that end a word.
static bool ends_word(char c) {
    return c == ' ' || c == '\t' || c == '<' || c == '>';
}

// Reads the operator at line[i], if there is one, digits first.
static bool read_operator(const std::string& line, size_t& i, std::vector<Token>& out) {
    size_t n = line.size();
    size_t j = i;
    while (j < n && std::isdigit(static_cast<unsigned char>(line[j]))) {
        ++j;
    }
    if (j == n || (line[j] != '<' && line[j] != '>')) {
        return false;  // a word (that may start with digits)
    }
    Token t;
    t.kind = Token::Op;
    if (j > i) {
        if (j - i > 9) {
            throw SyntaxError("fd number too big: " + line.substr(i, j - i));
        }
        t.fd = std::stoi(line.substr(i, j - i));  // at most 9 digits: it fits
    }
    bool two = j + 1 < n && (line[j + 1] == '&' || (line[j] == '>' && line[j + 1] == '>'));
    t.text = line.substr(j, two ? 2 : 1);
    i = j + t.text.size();
    out.push_back(t);
    return true;
}

std::vector<Token> tokenize(const std::string& line) {
    std::vector<Token> out;
    size_t i = 0;
    size_t n = line.size();
    while (i < n) {
        char c = line[i];
        if (c == ' ' || c == '\t') {
            ++i;
            continue;
        }
        if (c == '#') {
            break;  // a comment, to the end of the line
        }
        if (read_operator(line, i, out)) {
            continue;
        }
        size_t start = i;
        while (i < n && !ends_word(line[i])) {
            ++i;
        }
        out.push_back(Token{Token::Word, line.substr(start, i - start), -1});
    }
    return out;
}
