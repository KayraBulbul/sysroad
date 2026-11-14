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
    }
    return true;
}
