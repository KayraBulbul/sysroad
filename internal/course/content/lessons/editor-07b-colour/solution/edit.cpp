#include <iterator>

#include "kite.hpp"

namespace {

auto at(Editor& e, int y) {
    return e.rows.begin() + y;
}

bool past_the_end(const Editor& e) {
    return static_cast<std::size_t>(e.cy) == e.rows.size();
}

}  // namespace

void insert_char(Editor& e, char c) {
    if (past_the_end(e)) {
        e.rows.push_back(Row{});  // typing on the line past the end adds it
    }
    e.rows[e.cy].chars.insert(static_cast<std::size_t>(e.cx), 1, c);
    refresh_row(e, e.cy);
    ++e.cx;
    e.dirty = true;
}

void insert_newline(Editor& e) {
    if (past_the_end(e)) {
        e.rows.push_back(Row{});
        refresh_row(e, e.cy);
    } else {
        Row& row = e.rows[e.cy];
        Row rest{row.chars.substr(static_cast<std::size_t>(e.cx)), "", {}};
        row.chars.resize(static_cast<std::size_t>(e.cx));
        // Last: inserting can move every row, and `row` with them.
        e.rows.insert(std::next(at(e, e.cy)), std::move(rest));
        refresh_row(e, e.cy);
        refresh_row(e, e.cy + 1);
    }
    ++e.cy;
    e.cx = 0;
    e.dirty = true;
}

void delete_char(Editor& e) {
    if (past_the_end(e) || (e.cx == 0 && e.cy == 0)) {
        return;
    }
    if (e.cx > 0) {
        e.rows[e.cy].chars.erase(static_cast<std::size_t>(e.cx - 1), 1);
        refresh_row(e, e.cy);
        --e.cx;
        e.dirty = true;
        return;
    }
    // At the start of a line: join it onto the end of the one above.
    Row& above = e.rows[e.cy - 1];
    e.cx = static_cast<int>(above.chars.size());
    above.chars += e.rows[e.cy].chars;
    e.rows.erase(at(e, e.cy));
    --e.cy;
    refresh_row(e, e.cy);
    e.dirty = true;
}

void delete_forward(Editor& e) {
    if (past_the_end(e)) {
        return;
    }
    Row& row = e.rows[e.cy];
    if (static_cast<std::size_t>(e.cx) < row.chars.size()) {
        row.chars.erase(static_cast<std::size_t>(e.cx), 1);
    } else if (static_cast<std::size_t>(e.cy) + 1 < e.rows.size()) {
        row.chars += e.rows[e.cy + 1].chars;  // join the next line on
        e.rows.erase(at(e, e.cy + 1));
    } else {
        return;
    }
    refresh_row(e, e.cy);
    e.dirty = true;
}
