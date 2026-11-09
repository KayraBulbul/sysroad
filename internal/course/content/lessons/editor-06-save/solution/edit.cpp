// The course's version of what you wrote last lesson, now marking the file
// as changed. You won't need to change this file.
#include <iterator>

#include "editor.hpp"

namespace {
auto at(Editor& e, int y) {
    return e.rows.begin() + y;
}
}  // namespace

void insert_char(Editor& e, char c) {
    if (static_cast<std::size_t>(e.cy) == e.rows.size()) {
        e.rows.push_back(Row{});  // typing on the line past the end adds it
    }
    Row& row = e.rows[e.cy];
    row.chars.insert(static_cast<std::size_t>(e.cx), 1, c);
    update_render(row);
    ++e.cx;
    e.dirty = true;
}

void insert_newline(Editor& e) {
    if (static_cast<std::size_t>(e.cy) == e.rows.size()) {
        e.rows.push_back(Row{});
    } else {
        Row& row = e.rows[e.cy];
        Row rest{row.chars.substr(static_cast<std::size_t>(e.cx)), ""};
        row.chars.resize(static_cast<std::size_t>(e.cx));
        update_render(row);
        update_render(rest);
        e.rows.insert(std::next(at(e, e.cy)), std::move(rest));
    }
    ++e.cy;
    e.cx = 0;
    e.dirty = true;
}

void delete_char(Editor& e) {
    if (static_cast<std::size_t>(e.cy) == e.rows.size() || (e.cx == 0 && e.cy == 0)) {
        return;
    }
    if (e.cx > 0) {
        Row& row = e.rows[e.cy];
        row.chars.erase(static_cast<std::size_t>(e.cx - 1), 1);
        update_render(row);
        --e.cx;
        e.dirty = true;
        return;
    }
    // At the start of a line: join it onto the end of the one above.
    Row& above = e.rows[e.cy - 1];
    e.cx = static_cast<int>(above.chars.size());
    above.chars += e.rows[e.cy].chars;
    update_render(above);
    e.rows.erase(at(e, e.cy));
    --e.cy;
    e.dirty = true;
}
