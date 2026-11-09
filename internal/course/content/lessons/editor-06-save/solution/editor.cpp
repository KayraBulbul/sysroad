// What each key does, prompts, and opening files. You won't need to change
// this file.
#include "editor.hpp"

#include <format>
#include <fstream>
#include <optional>

#include "keys.hpp"
#include "terminal.hpp"

void set_status(Editor& e, std::string msg) {
    e.status_msg = std::move(msg);
    e.status_time = std::time(nullptr);
}

void open_file(Editor& e, const std::string& path) {
    e.filename = path;
    std::ifstream in(path, std::ios::binary);
    if (in) {
        e.rows = read_rows(in);
    } else {
        set_status(e, "New file");
    }
}

// Asks for a line of text in the message bar. Returns nothing if Escape
// cancels it.
static std::optional<std::string> prompt(Editor& e, KeyReader& keys, std::string_view label) {
    std::string answer;
    while (true) {
        set_status(e, std::format("{}{}", label, answer));
        scroll(e);
        write_all(draw_screen(e));
        int key = keys.next();
        if (key == kEsc) {
            set_status(e, "");
            return std::nullopt;
        }
        if (key == kBackspace || key == ctrl_key('h') || key == kDelete) {
            if (!answer.empty()) {
                answer.pop_back();
            }
        } else if (key == kEnter) {
            if (!answer.empty()) {
                set_status(e, "");
                return answer;
            }
        } else if (key >= 32 && key < 256 && key != kBackspace) {
            answer += static_cast<char>(key);
        }
    }
}

bool process_key(Editor& e, KeyReader& keys, int key) {
    switch (key) {
    case ctrl_key('q'):
        if (e.dirty && e.quit_times > 0) {
            set_status(e,
                       std::format("Unsaved changes! Press Ctrl-Q {} more time{} to quit anyway.",
                                   e.quit_times, e.quit_times == 1 ? "" : "s"));
            --e.quit_times;
            return true;
        }
        return false;
    case ctrl_key('s'):
        if (e.filename.empty()) {
            auto name = prompt(e, keys, "Save as (Esc to cancel): ");
            if (!name) {
                set_status(e, "Not saved");
                break;
            }
            e.filename = *name;
        }
        save_file(e);
        break;
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
    e.quit_times = kQuitTimes;  // any other key starts the count again
    return true;
}
