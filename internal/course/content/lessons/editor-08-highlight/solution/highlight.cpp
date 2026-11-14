#include <algorithm>
#include <cctype>
#include <cstring>

#include "kite.hpp"

bool is_separator(char c) {
    return std::isspace(static_cast<unsigned char>(c)) || c == '\0' ||
           std::strchr(",.()+-/*=~%<>[]{};:!&|^?", c) != nullptr;
}

namespace {

bool starts_with_at(const std::string& s, std::size_t i, std::string_view word) {
    return s.compare(i, word.size(), word) == 0;
}

// The length of a keyword or type name starting at i, as a whole word, or 0.
std::size_t word_at(const std::string& s, std::size_t i, std::string_view word) {
    std::size_t end = i + word.size();
    if (starts_with_at(s, i, word) && (end == s.size() || is_separator(s[end]))) {
        return word.size();
    }
    return 0;
}

}  // namespace

bool highlight_row(Row& row, bool in_comment) {
    const std::string& s = row.render;
    row.hl.assign(s.size(), Hl::Normal);
    bool prev_sep = true;  // the start of the line counts as a separator
    char in_string = 0;    // the quote that opened the string, or 0
    bool preproc = false;  // in a #directive line
    std::size_t i = 0;
    while (i < s.size()) {
        char c = s[i];
        Hl prev = i > 0 ? row.hl[i - 1] : Hl::Normal;

        if (!in_string && !in_comment && starts_with_at(s, i, "//")) {
            std::fill(row.hl.begin() + static_cast<long>(i), row.hl.end(), Hl::Comment);
            break;
        }
        if (in_comment) {
            row.hl[i] = Hl::MlComment;
            if (starts_with_at(s, i, "*/")) {
                row.hl[i + 1] = Hl::MlComment;
                i += 2;
                in_comment = false;
                prev_sep = true;
            } else {
                ++i;
            }
            continue;
        }
        if (!in_string && starts_with_at(s, i, "/*")) {
            row.hl[i] = row.hl[i + 1] = Hl::MlComment;
            i += 2;
            in_comment = true;
            continue;
        }
        if (in_string) {
            row.hl[i] = Hl::String;
            if (c == '\\' && i + 1 < s.size()) {  // an escape: the next char is part of it
                row.hl[i + 1] = Hl::String;
                i += 2;
                continue;
            }
            if (c == in_string) {
                in_string = 0;
            }
            ++i;
            prev_sep = true;
            continue;
        }
        if (c == '"' || c == '\'') {
            in_string = c;
            row.hl[i] = Hl::String;
            ++i;
            continue;
        }
        auto uc = static_cast<unsigned char>(c);
        if ((std::isdigit(uc) && (prev_sep || prev == Hl::Number)) ||
            ((c == '.' || std::isalnum(uc)) && prev == Hl::Number)) {
            row.hl[i] = Hl::Number;
            ++i;
            prev_sep = false;
            continue;
        }
        if (c == '#' && s.find_first_not_of(" \t") == i) {
            preproc = true;
        }
        if (prev_sep && !preproc) {
            std::size_t len = 0;
            Hl kind = Hl::Keyword;
            for (std::string_view k : kKeywords) {
                if ((len = word_at(s, i, k)) > 0) {
                    break;
                }
            }
            if (len == 0) {
                kind = Hl::Type;
                for (std::string_view t : kTypes) {
                    if ((len = word_at(s, i, t)) > 0) {
                        break;
                    }
                }
            }
            if (len > 0) {
                std::fill(row.hl.begin() + static_cast<long>(i), row.hl.begin() + static_cast<long>(i + len), kind);
                i += len;
                prev_sep = false;
                continue;
            }
        }
        row.hl[i] = preproc ? Hl::Preproc : Hl::Normal;
        prev_sep = is_separator(c);
        ++i;
    }
    return in_comment;
}

void update_syntax(Editor& e, int y) {
    bool in = y > 0 && e.rows[y - 1].open_comment;
    for (auto r = static_cast<std::size_t>(y); r < e.rows.size(); ++r) {
        Row& row = e.rows[r];
        // A later row that was coloured from the same starting state is
        // still right, and so is everything after it.
        if (r > static_cast<std::size_t>(y) && row.hl_done && row.hl_in == in) {
            break;
        }
        row.hl_in = in;
        row.hl_done = true;
        if (e.highlight) {
            row.open_comment = highlight_row(row, in);
        } else {
            row.hl.assign(row.render.size(), Hl::Normal);
            row.open_comment = false;
        }
        in = row.open_comment;
    }
}

void refresh_row(Editor& e, int y) {
    update_render(e.rows[y]);
    e.rows[y].hl_done = false;
    update_syntax(e, y);
}

void select_syntax(Editor& e) {
    e.highlight = false;
    auto dot = e.filename.rfind('.');
    if (dot != std::string::npos) {
        std::string_view ext = std::string_view(e.filename).substr(dot);
        for (std::string_view c : {".c", ".h", ".cc", ".cpp", ".cxx", ".hh", ".hpp", ".hxx"}) {
            e.highlight = e.highlight || ext == c;
        }
    }
    for (Row& row : e.rows) {
        update_render(row);
        row.hl_done = false;
    }
    if (!e.rows.empty()) {
        update_syntax(e, 0);
    }
}
