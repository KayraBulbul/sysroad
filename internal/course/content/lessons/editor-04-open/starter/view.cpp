#include <stdexcept>

#include "editor.hpp"

std::vector<Row> read_rows(std::istream& in) {
    // TODO: one Row per line, without "\n" or "\r\n", render filled in.
    (void)in;
    throw std::logic_error("read_rows isn't written yet");
}

void update_render(Row& row) {
    // TODO: chars, with each tab turned into spaces up to the next tab stop.
    row.render = row.chars;
}

int cx_to_rx(const Row& row, int cx) {
    // TODO: count columns the way update_render lays them out.
    (void)row;
    return cx;
}

void scroll(Editor& e) {
    // TODO: set e.rx, then move rowoff and coloff so the cursor is on screen.
    (void)e;
}
