// The course's version of this file. You won't need to change it.
#include <cctype>
#include <string>

#include "shell.hpp"

// Characters that end an unquoted word.
static bool ends_word(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '|' || c == '&' || c == ';' || c == '<' ||
           c == '>';
}

// Reads the operator at line[i], if there is one.
static bool read_operator(const std::string& line, size_t& i, std::vector<Token>& out) {
    size_t n = line.size();
    size_t j = i;
    while (j < n && std::isdigit(static_cast<unsigned char>(line[j]))) {
        ++j;
    }
    Token t;
    t.kind = Token::Op;
    if (j < n && (line[j] == '<' || line[j] == '>')) {
        if (j - i > 9) {
            throw SyntaxError("fd number too big: " + line.substr(i, j - i));
        }
        if (j > i) {
            t.fd = std::stoi(line.substr(i, j - i));  // at most 9 digits: fits an int
        }
        bool two = j + 1 < n && (line[j + 1] == '&' || (line[j] == '>' && line[j + 1] == '>'));
        t.text = line.substr(j, two ? 2 : 1);
        i = j + t.text.size();
        out.push_back(t);
        return true;
    }
    char c = line[i];
    if (c == '|' || c == '&') {
        bool two = i + 1 < n && line[i + 1] == c;
        t.text = line.substr(i, two ? 2 : 1);
    } else if (c == ';') {
        t.text = ";";
    } else {
        return false;
    }
    i += t.text.size();
    out.push_back(t);
    return true;
}

std::vector<Token> tokenize(const std::string& line) {
    std::vector<Token> out;
    size_t i = 0;
    size_t n = line.size();
    while (i < n) {
        char c = line[i];
        if (c == ' ' || c == '\t' || c == '\n') {
            ++i;
            continue;
        }
        if (c == '#') {
            break;  // a comment, to the end of the line
        }
        if (read_operator(line, i, out)) {
            continue;
        }
        Token word;
        size_t start = i;
        while (i < n && !ends_word(line[i])) {
            ++i;
        }
        word.text = line.substr(start, i - start);
        out.push_back(word);
    }
    return out;
}
