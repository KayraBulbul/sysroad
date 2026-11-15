#include <algorithm>
#include <cctype>

#include "conch.hpp"

// Characters that end a word.
static bool ends_word(const std::string& line, size_t i) {
    char c = line[i];
    return c == ' ' || c == '\t' || c == '<' || c == '>' || c == '|' || c == ';' || c == '&';
}

// Reads the operator at line[i], if there is one, digits first.
static bool read_operator(const std::string& line, size_t& i, std::vector<Token>& out) {
    size_t n = line.size();
    size_t j = i;
    while (j < n && std::isdigit(static_cast<unsigned char>(line[j]))) {
        ++j;
    }
    Token t;
    t.kind = Token::Op;
    if (j == i) {  // no digits: maybe | || && or ;
        char c = line[i];
        bool twice = i + 1 < n && line[i + 1] == c;
        if (c == '|' || c == '&') {
            t.text = line.substr(i, twice ? 2 : 1);
        } else if (c == ';') {
            t.text = ";";
        }
        if (!t.text.empty()) {
            i += t.text.size();
            out.push_back(t);
            return true;
        }
    }
    if (j == n || (line[j] != '<' && line[j] != '>')) {
        return false;  // a word (that may start with digits)
    }
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
        while (i < n && !ends_word(line, i)) {
            c = line[i];
            if (c == '\\') {
                i = std::min(i + 2, n);  // the next character, whatever it is
            } else if (c == '\'') {
                size_t close = line.find('\'', i + 1);
                if (close == std::string::npos) {
                    throw SyntaxError("unterminated '");
                }
                i = close + 1;
            } else if (c == '"') {
                ++i;
                while (i < n && line[i] != '"') {
                    i += line[i] == '\\' ? 2 : 1;
                }
                if (i >= n) {
                    throw SyntaxError("unterminated \"");
                }
                ++i;
            } else {
                ++i;
            }
        }
        out.push_back(Token{Token::Word, line.substr(start, i - start), -1});
    }
    return out;
}
