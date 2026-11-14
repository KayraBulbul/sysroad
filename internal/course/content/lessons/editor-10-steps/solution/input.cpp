#include <format>

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

// A key while a prompt is open: it edits the answer, or ends the prompt.
void prompt_key(Editor& e, int key) {
    Prompt& p = *e.prompt;
    if (key == kEsc || (key == kEnter && !p.answer.empty())) {
        Prompt done = std::move(p);
        e.prompt.reset();  // first: done may ask something new
        if (key == kEsc) {
            done.done(e, std::nullopt);
        } else {
            done.done(e, done.answer);
        }
        return;
    }
    if (key == kBackspace || key == ctrl_key('h') || key == kDelete) {
        if (!p.answer.empty()) {
            p.answer.pop_back();
        }
    } else if (key >= 32 && key < 256 && key != kBackspace) {
        p.answer += static_cast<char>(key);
    }
    if (p.on_key) {
        p.on_key(e, p.answer, key);
    }
}

// Ctrl-S: saves, asking for a name first if the file has none.
void save(Editor& e) {
    if (!e.filename.empty()) {
        save_file(e);
        return;
    }
    Prompt p;
    p.label = "Save as: ";
    p.done = [](Editor& e, std::optional<std::string> name) {
        if (!name) {
            set_status(e, "Not saved");
            return;
        }
        e.filename = *name;
        select_syntax(e);  // a new name can make it C++
        save_file(e);
    };
    e.prompt = std::move(p);
}

}  // namespace

bool process_key(Editor& e, int key) {
    if (e.prompt) {
        prompt_key(e, key);
        return true;
    }
    if (key != ctrl_key('q')) {
        e.quit_times = kQuitTimes;  // any other key starts the count again
    }
    switch (key) {
    case ctrl_key('q'):
        if (e.dirty && e.quit_times > 0) {
            set_status(e, std::format("Unsaved changes! Press Ctrl-Q {} more time{} to quit anyway.", e.quit_times,
                                      e.quit_times == 1 ? "" : "s"));
            --e.quit_times;
            return true;
        }
        return false;
    case ctrl_key('s'):
        save(e);
        break;
    case ctrl_key('f'):
        start_search(e);
        e.history.sealed = true;  // it moves the cursor
        break;
    case ctrl_key('z'):
        if (!undo(e)) {
            set_status(e, "Nothing to undo");
        }
        break;
    case ctrl_key('y'):
        if (!redo(e)) {
            set_status(e, "Nothing to redo");
        }
        break;
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
        e.history.sealed = true;  // typing somewhere else is a new step
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
