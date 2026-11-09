// The course's version of what you wrote last lesson. You won't need to
// change this file.
#include <string>

#include "editor.hpp"

std::vector<Row> read_rows(std::istream& in) {
    std::vector<Row> rows;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        Row row{line, ""};
        update_render(row);
        rows.push_back(std::move(row));
    }
    return rows;
}

void update_render(Row& row) {
    row.render.clear();
    for (char c : row.chars) {
        if (c == '\t') {
            row.render += ' ';
            while (row.render.size() % kTabStop != 0) {
                row.render += ' ';
            }
        } else {
            row.render += c;
        }
    }
}

int cx_to_rx(const Row& row, int cx) {
    int rx = 0;
    for (int j = 0; j < cx && static_cast<std::size_t>(j) < row.chars.size(); ++j) {
        if (row.chars[j] == '\t') {
            rx += (kTabStop - 1) - (rx % kTabStop);
        }
        ++rx;
    }
    return rx;
}

void scroll(Editor& e) {
    e.rx = 0;
    if (static_cast<std::size_t>(e.cy) < e.rows.size()) {
        e.rx = cx_to_rx(e.rows[e.cy], e.cx);
    }
    if (e.cy < e.rowoff) {  // above the screen
        e.rowoff = e.cy;
    }
    if (e.cy >= e.rowoff + e.screen_rows) {  // below it
        e.rowoff = e.cy - e.screen_rows + 1;
    }
    if (e.rx < e.coloff) {
        e.coloff = e.rx;
    }
    if (e.rx >= e.coloff + e.screen_cols) {
        e.coloff = e.rx - e.screen_cols + 1;
    }
}
