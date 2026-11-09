#include "editor.hpp"

void record(History& h, Edit edit) {
    // TODO: join the last step when it carries on from it; empty redo.
    h.undo.push_back(std::move(edit));
}

bool undo(Editor& e) {
    // TODO
    (void)e;
    return false;
}

bool redo(Editor& e) {
    // TODO
    (void)e;
    return false;
}
