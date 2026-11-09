// What each key does, prompts, search, and opening files. You won't need to change
// this file.
#include "editor.hpp"

#include <format>
#include <fstream>
#include <functional>
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
    select_syntax(e);
}

using PromptCallback = std::function<void(const std::string& answer, int key)>;

// Asks for a line of text in the message bar. Returns nothing if Escape
// cancels it. on_key, if given, hears about every key, after the answer
// has been updated.
static std::optional<std::string> prompt(Editor& e, KeyReader& keys, std::string_view label,
                                         const PromptCallback& on_key = nullptr) {
    std::string answer;
    while (true) {
        set_status(e, std::format("{}{}", label, answer));
        scroll(e);
        write_all(draw_screen(e));
        int key = keys.next();
        if (key == kBackspace || key == ctrl_key('h') || key == kDelete) {
            if (!answer.empty()) {
                answer.pop_back();
            }
        } else if (key >= 32 && key < 256 && key != kBackspace) {
            answer += static_cast<char>(key);
        }
        if (on_key) {
            on_key(answer, key);
        }
        if (key == kEsc) {
            set_status(e, "");
            return std::nullopt;
        }
        if (key == kEnter && !answer.empty()) {
            set_status(e, "");
            return answer;
        }
    }
}

// Ctrl-F: search as you type. The arrows go to the next or previous match,
// Enter stays there, Escape goes back to where you were.
static void search(Editor& e, KeyReader& keys) {
    Editor before = e;
    Pos start{e.cy, e.cx};
    std::optional<Pos> match;
    // The match is shown by colouring it; this puts back the colours it had.
    int marked_row = -1;
    std::vector<Hl> saved_hl;
    auto unmark = [&] {
        if (marked_row >= 0 && static_cast<std::size_t>(marked_row) < e.rows.size()) {
            e.rows[marked_row].hl = saved_hl;
        }
        marked_row = -1;
    };
    auto on_key = [&](const std::string& query, int key) {
        unmark();
        if (key == kEnter || key == kEsc) {
            return;
        }
        Pos from = start;
        int dir = 1;
        if (match && (key == kArrowRight || key == kArrowDown)) {
            from = {match->row, match->col + 1};
        } else if (match && (key == kArrowLeft || key == kArrowUp)) {
            from = {match->row, match->col - 1};
            dir = -1;
        }
        match = find(e.rows, query, from, dir);
        if (match) {
            e.cy = match->row;
            e.cx = match->col;
            e.rowoff = static_cast<int>(e.rows.size());  // scroll puts the match at the top
            Row& row = e.rows[match->row];
            marked_row = match->row;
            saved_hl = row.hl;
            int from = cx_to_rx(row, match->col);
            int to = cx_to_rx(row, match->col + static_cast<int>(query.size()));
            for (int i = from; i < to && static_cast<std::size_t>(i) < row.hl.size(); ++i) {
                row.hl[i] = Hl::Match;
            }
        } else {
            e.cx = before.cx;
            e.cy = before.cy;
            e.rowoff = before.rowoff;
            e.coloff = before.coloff;
        }
    };
    if (!prompt(e, keys, "Search (arrows: next/previous, Esc: cancel): ", on_key)) {
        e.cx = before.cx;
        e.cy = before.cy;
        e.rowoff = before.rowoff;
        e.coloff = before.coloff;
    } else if (!match) {
        set_status(e, "Not found");
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
            select_syntax(e);
        }
        save_file(e);
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
    case ctrl_key('f'):
        search(e, keys);
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
