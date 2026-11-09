#pragma once
// Editors holding text, and the text they hold.
#include <initializer_list>
#include <string>

#include "editor.hpp"

inline Editor with_lines(std::initializer_list<std::string> lines) {
    Editor e;
    for (const auto& l : lines) {
        Row r{l, ""};
        update_render(r);
        e.rows.push_back(r);
    }
    return e;
}

// The lines joined with '|', and each render checked against its chars.
inline std::string text(const Editor& e) {
    std::string out;
    for (std::size_t i = 0; i < e.rows.size(); ++i) {
        Row r{e.rows[i].chars, ""};
        update_render(r);
        if (r.render != e.rows[i].render) {
            return "render of line " + std::to_string(i) + " is stale";
        }
        out += (i > 0 ? "|" : "") + e.rows[i].chars;
    }
    return out;
}

inline void type(Editor& e, const std::string& s) {
    for (char c : s) {
        if (c == '\n') {
            insert_newline(e);
        } else {
            insert_char(e, c);
        }
    }
}
