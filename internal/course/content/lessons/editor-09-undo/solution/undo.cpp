#include "kite.hpp"

void record(History& h, Edit edit) {
    h.redo.clear();  // a new edit: what was undone can't be redone any more
    h.undo.push_back(std::move(edit));
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
    return true;
}
