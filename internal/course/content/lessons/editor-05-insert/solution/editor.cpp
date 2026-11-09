// What each key does. You won't need to change this file.
#include "editor.hpp"
#include "keys.hpp"

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
    case kDelete: {
        // Delete is Backspace one place to the right, unless there's
        // nothing to the right.
        auto y = static_cast<std::size_t>(e.cy);
        bool at_end =
            y >= e.rows.size() ||
            (y + 1 == e.rows.size() && static_cast<std::size_t>(e.cx) == e.rows[y].chars.size());
        if (!at_end) {
            move_cursor(e, kArrowRight);
            delete_char(e);
        }
        break;
    }
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
        // Printable characters, tabs and UTF-8 bytes; not other
        // control keys, or keys like F5.
        if (key == kTab || (key >= 32 && key < 256 && key != kBackspace)) {
            insert_char(e, static_cast<char>(key));
        }
        break;
    }
    return true;
}
