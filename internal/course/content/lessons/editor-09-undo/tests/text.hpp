#pragma once
#include <initializer_list>
#include <string>

#include "editor.hpp"

inline Editor with_lines(std::initializer_list<std::string> lines) {
    Editor e;
    for (const auto& l : lines) {
        Row r;
        r.chars = l;
        update_render(r);
        e.rows.push_back(r);
    }
    return e;
}

// The lines joined with '|', and the cursor as "@row:col".
inline std::string text(const Editor& e) {
    std::string out;
    for (std::size_t i = 0; i < e.rows.size(); ++i) {
        out += (i > 0 ? "|" : "") + e.rows[i].chars;
    }
    return out + " @" + std::to_string(e.cy) + ":" + std::to_string(e.cx);
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

inline void backspace(Editor& e, int times) {
    for (int i = 0; i < times; ++i) {
        delete_char(e);
    }
}
