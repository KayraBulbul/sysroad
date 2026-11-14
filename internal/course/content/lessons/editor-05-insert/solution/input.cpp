#include "kite.hpp"

namespace {

// The length of row y, or 0 for the line past the end.
int row_len(const Editor& e, int y) {
    if (y < 0 || static_cast<std::size_t>(y) >= e.rows.size()) {
        return 0;
    }
    return static_cast<int>(e.rows[y].chars.size());
}

int last_row(const Editor& e) {
    return static_cast<int>(e.rows.size());  // the line just past the end
}

void move_cursor(Editor& e, int key) {
    switch (key) {
    case kArrowLeft:
        if (e.cx > 0) {
            --e.cx;
        } else if (e.cy > 0) {  // wrap to the end of the line above
            --e.cy;
            e.cx = row_len(e, e.cy);
        }
        break;
    case kArrowRight:
        if (e.cy < last_row(e)) {
            if (e.cx < row_len(e, e.cy)) {
                ++e.cx;
            } else {  // wrap to the start of the next line
                ++e.cy;
                e.cx = 0;
            }
        }
        break;
    case kArrowUp:
        if (e.cy > 0) {
            --e.cy;
        }
        break;
    case kArrowDown:
        if (e.cy < last_row(e)) {
            ++e.cy;
        }
        break;
    case kHome:
        e.cx = 0;
        break;
    case kEnd:
        e.cx = row_len(e, e.cy);
        break;
    case kPageUp:
    case kPageDown:
        for (int i = 0; i < e.screen_rows; ++i) {
            move_cursor(e, key == kPageUp ? kArrowUp : kArrowDown);
        }
        break;
    }
    // Up and down can land on a shorter line.
    if (e.cx > row_len(e, e.cy)) {
        e.cx = row_len(e, e.cy);
    }
}

}  // namespace

bool process_key(Editor& e, int key) {
    switch (key) {
    case ctrl_key('q'):
        return false;
    case kEnter:
        insert_newline(e);
        break;
    case kBackspace:
    case ctrl_key('h'):
        delete_char(e);
        break;
    case kDelete:
        delete_forward(e);
        break;
    case kArrowLeft:
    case kArrowRight:
    case kArrowUp:
    case kArrowDown:
    case kHome:
    case kEnd:
    case kPageUp:
    case kPageDown:
        move_cursor(e, key);
        break;
    default:
        // Printable characters, tabs and UTF-8 bytes; not other control
        // keys, or keys like F5.
        if (key == kTab || (key >= 32 && key < 256 && key != kBackspace)) {
            insert_char(e, static_cast<char>(key));
        }
        break;
    }
    return true;
}
