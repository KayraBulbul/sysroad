#include "editor.hpp"

bool is_separator(char c) {
    // TODO
    (void)c;
    return false;
}

bool highlight_row(Row& row, bool in_comment) {
    // TODO: walk row.render left to right, deciding what each character is.
    row.hl.assign(row.render.size(), Hl::Normal);
    return in_comment;
}
