#pragma once
// Makes an editor holding the given lines.
#include <initializer_list>
#include <string>

#include "editor.hpp"

inline Editor with_lines(std::initializer_list<std::string> lines, int screen_rows = 10) {
    Editor e;
    e.screen_rows = screen_rows;
    e.screen_cols = 40;
    for (const auto& l : lines) {
        e.rows.push_back(Row{l, l});
    }
    return e;
}

inline void press(Editor& e, int key, int times = 1) {
    for (int i = 0; i < times; ++i) {
        move_cursor(e, key);
    }
}
