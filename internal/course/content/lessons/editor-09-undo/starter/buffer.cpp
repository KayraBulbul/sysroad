// Inserting and erasing any text, line breaks included: what undo and redo
// are made of. You won't need to change this file.
#include "editor.hpp"

Pos insert_text(Editor& e, Pos at, std::string_view text) {
    if (static_cast<std::size_t>(at.row) == e.rows.size()) {
        e.rows.emplace_back();
    }
    auto col = static_cast<std::size_t>(at.col);
    std::string tail = e.rows[at.row].chars.substr(col);
    e.rows[at.row].chars.resize(col);
    Pos end = at;
    std::size_t start = 0;
    while (true) {
        std::size_t nl = text.find('\n', start);
        std::string_view part = text.substr(start, nl == std::string_view::npos ? nl : nl - start);
        e.rows[end.row].chars += part;
        end.col = static_cast<int>(e.rows[end.row].chars.size());
        if (nl == std::string_view::npos) {
            break;
        }
        e.rows.insert(e.rows.begin() + end.row + 1, Row{});
        ++end.row;
        start = nl + 1;
    }
    e.rows[end.row].chars += tail;
    for (int y = at.row; y <= end.row; ++y) {
        update_render(e.rows[y]);
        e.rows[y].hl_done = false;
    }
    update_syntax(e, at.row);
    return end;
}

void erase_text(Editor& e, Pos at, std::size_t n) {
    auto col = static_cast<std::size_t>(at.col);
    while (n > 0) {
        std::string& chars = e.rows[at.row].chars;
        std::size_t here = std::min(n, chars.size() - col);
        chars.erase(col, here);
        n -= here;
        if (n > 0 && static_cast<std::size_t>(at.row) + 1 < e.rows.size()) {
            chars += e.rows[at.row + 1].chars;  // the line break: join the next line on
            e.rows.erase(e.rows.begin() + at.row + 1);
            --n;
        } else {
            break;
        }
    }
    refresh_row(e, at.row);
}
