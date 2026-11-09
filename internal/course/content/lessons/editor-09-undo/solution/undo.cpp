#include "editor.hpp"

namespace {

bool one_line(const std::string& s) {
    return s.find('\n') == std::string::npos;
}

// Whether edit carries on from last, so they're one step.
bool joins(const Edit& last, const Edit& edit) {
    if (last.kind != edit.kind || !one_line(last.text) || !one_line(edit.text) ||
        last.at.row != edit.at.row) {
        return false;
    }
    if (edit.kind == Edit::Kind::Insert) {
        bool continues = edit.at.col == last.at.col + static_cast<int>(last.text.size());
        bool new_word = last.text.back() == ' ' && edit.text.front() != ' ';
        return continues && !new_word;
    }
    bool backspace = edit.at.col + static_cast<int>(edit.text.size()) == last.at.col;
    bool del = edit.at.col == last.at.col;
    return backspace || del;
}

}  // namespace

void record(History& h, Edit edit) {
    h.redo.clear();
    if (!h.sealed && !h.undo.empty() && joins(h.undo.back(), edit)) {
        Edit& last = h.undo.back();
        if (edit.kind == Edit::Kind::Insert) {
            last.text += edit.text;
        } else if (edit.at.col < last.at.col) {  // Backspace: it grows to the left
            last.at = edit.at;
            last.text = edit.text + last.text;
        } else {  // Delete: it grows to the right
            last.text += edit.text;
        }
    } else {
        h.undo.push_back(std::move(edit));
    }
    h.sealed = false;
}

bool undo(Editor& e) {
    History& h = e.history;
    if (h.undo.empty()) {
        return false;
    }
    Edit edit = std::move(h.undo.back());
    h.undo.pop_back();
    if (edit.kind == Edit::Kind::Insert) {
        erase_text(e, edit.at, edit.text.size());
    } else {
        insert_text(e, edit.at, edit.text);
    }
    e.cy = edit.cursor.row;
    e.cx = edit.cursor.col;
    e.dirty = true;
    h.redo.push_back(std::move(edit));
    h.sealed = true;  // typing next starts a step of its own
    return true;
}

bool redo(Editor& e) {
    History& h = e.history;
    if (h.redo.empty()) {
        return false;
    }
    Edit edit = std::move(h.redo.back());
    h.redo.pop_back();
    Pos cursor = edit.at;
    if (edit.kind == Edit::Kind::Insert) {
        cursor = insert_text(e, edit.at, edit.text);
    } else {
        erase_text(e, edit.at, edit.text.size());
    }
    e.cy = cursor.row;
    e.cx = cursor.col;
    e.dirty = true;
    h.undo.push_back(std::move(edit));  // straight back: not record, which would empty redo
    h.sealed = true;
    return true;
}
