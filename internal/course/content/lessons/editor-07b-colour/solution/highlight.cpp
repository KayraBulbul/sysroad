#include <cctype>
#include <cstring>

#include "kite.hpp"

bool is_separator(char c) {
    return std::isspace(static_cast<unsigned char>(c)) || c == '\0' ||
           std::strchr(",.()+-/*=~%<>[]{};:!&|^?", c) != nullptr;
}

void highlight_row(Row& row) {
    const std::string& s = row.render;
    row.hl.assign(s.size(), Hl::Normal);
    bool prev_sep = true;  // the start of the line counts as a separator
    char in_string = 0;    // the quote that opened the string, or 0
    std::size_t i = 0;
    while (i < s.size()) {
        char c = s[i];
        Hl prev = i > 0 ? row.hl[i - 1] : Hl::Normal;
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
        prev_sep = is_separator(c);
        ++i;
    }
}

void refresh_row(Editor& e, int y) {
    Row& row = e.rows[y];
    update_render(row);
    if (e.highlight) {
        highlight_row(row);
    } else {
        row.hl.assign(row.render.size(), Hl::Normal);
    }
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
    for (int y = 0; y < static_cast<int>(e.rows.size()); ++y) {
        refresh_row(e, y);
    }
}
