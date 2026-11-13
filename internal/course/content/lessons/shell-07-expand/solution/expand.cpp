#include <unistd.h>

#include <cctype>
#include <string>

#include "shell.hpp"

static bool name_start(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

static bool name_char(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

// Expands the $ at word[i] into out, and returns the index after it.
static size_t expand_dollar(const std::string& word, size_t i, const Shell& sh, std::string& out) {
    size_t n = word.size();
    char c = i + 1 < n ? word[i + 1] : '\0';
    if (c == '?') {
        out += std::to_string(sh.last_status);
        return i + 2;
    }
    if (c == '$') {
        out += std::to_string(getpid());
        return i + 2;
    }
    if (c == '!') {
        if (sh.last_background != 0) {
            out += std::to_string(sh.last_background);
        }
        return i + 2;
    }
    if (c == '{') {
        size_t close = word.find('}', i + 2);
        if (close == std::string::npos) {
            throw SyntaxError("missing }");
        }
        out += lookup_var(sh, word.substr(i + 2, close - i - 2)).value_or("");
        return close + 1;
    }
    if (name_start(c)) {
        size_t end = i + 1;
        while (end < n && name_char(word[end])) {
            ++end;
        }
        out += lookup_var(sh, word.substr(i + 1, end - i - 1)).value_or("");
        return end;
    }
    out += '$';  // a $ that starts nothing stays a $
    return i + 1;
}

std::optional<std::string> expand_word(const std::string& word, const Shell& sh) {
    std::string out;
    bool kept = false;  // quotes or plain text: the word stays even if empty
    size_t n = word.size();
    size_t i = 0;
    if (n > 0 && word[0] == '~' && (n == 1 || word[1] == '/')) {
        out += lookup_var(sh, "HOME").value_or("~");
        kept = true;
        i = 1;
    }
    while (i < n) {
        char c = word[i];
        if (c == '\\') {
            if (i + 1 < n) {
                out += word[i + 1];
            }
            i += 2;
            kept = true;
        } else if (c == '\'') {
            size_t close = word.find('\'', i + 1);
            out += word.substr(i + 1, close - i - 1);
            i = close + 1;
            kept = true;
        } else if (c == '"') {
            ++i;
            while (i < n && word[i] != '"') {
                if (word[i] == '\\' && i + 1 < n &&
                    std::string("\"\\$`").find(word[i + 1]) != std::string::npos) {
                    out += word[i + 1];
                    i += 2;
                } else if (word[i] == '$') {
                    i = expand_dollar(word, i, sh, out);
                } else {
                    out += word[i++];
                }
            }
            ++i;
            kept = true;
        } else if (c == '$') {
            size_t at = out.size();
            i = expand_dollar(word, i, sh, out);
            kept = kept || out.compare(at, std::string::npos, "$") == 0;
        } else {
            out += c;
            ++i;
            kept = true;
        }
    }
    if (!kept && out.empty()) {
        return std::nullopt;
    }
    return out;
}
