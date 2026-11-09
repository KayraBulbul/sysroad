// Typing, Enter, Backspace and Delete, built on insert_text and erase_text so
// that undo can take each one back. You won't need to change this file.
#include "editor.hpp"

namespace {

int len(const Editor& e, int y) {
    return static_cast<int>(e.rows[y].chars.size());
}

void insert(Editor& e, std::string text) {
    Pos before{e.cy, e.cx};
    Pos at = before;
    // On the line past the end, the text goes on a new line after the last.
    if (static_cast<std::size_t>(e.cy) == e.rows.size() && !e.rows.empty()) {
        at = {e.cy - 1, len(e, e.cy - 1)};
        text = "\n" + text;
    }
    Pos end = insert_text(e, at, text);
    record(e.history, Edit{Edit::Kind::Insert, at, text, before});
    e.cy = end.row;
    e.cx = end.col;
    e.dirty = true;
}

void erase(Editor& e, Pos at, char c) {
    Pos before{e.cy, e.cx};
    erase_text(e, at, 1);
    record(e.history, Edit{Edit::Kind::Erase, at, std::string(1, c), before});
    e.cy = at.row;
    e.cx = at.col;
    e.dirty = true;
}

}  // namespace

void insert_char(Editor& e, char c) {
    insert(e, std::string(1, c));
}

void insert_newline(Editor& e) {
    if (e.rows.empty()) {  // the first line of a new file
        e.rows.emplace_back();
        e.cy = 1;
        e.dirty = true;
        return;
    }
    insert(e, "\n");
}

void delete_char(Editor& e) {
    if (static_cast<std::size_t>(e.cy) == e.rows.size() || (e.cx == 0 && e.cy == 0)) {
        return;
    }
    if (e.cx > 0) {
        erase(e, {e.cy, e.cx - 1}, e.rows[e.cy].chars[e.cx - 1]);
    } else {
        erase(e, {e.cy - 1, len(e, e.cy - 1)}, '\n');
    }
}

void delete_forward(Editor& e) {
    auto y = static_cast<std::size_t>(e.cy);
    if (y >= e.rows.size()) {
        return;
    }
    if (e.cx < len(e, e.cy)) {
        erase(e, {e.cy, e.cx}, e.rows[y].chars[e.cx]);
    } else if (y + 1 < e.rows.size()) {
        erase(e, {e.cy, e.cx}, '\n');
    }
}
