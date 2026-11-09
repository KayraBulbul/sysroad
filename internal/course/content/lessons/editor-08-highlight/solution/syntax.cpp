// Keeping each row's colours up to date, and what the colours are. You won't
// need to change this file.
#include "editor.hpp"

void update_syntax(Editor& e, int y) {
    bool in = y > 0 && e.rows[y - 1].open_comment;
    for (auto r = static_cast<std::size_t>(y); r < e.rows.size(); ++r) {
        Row& row = e.rows[r];
        // A later row that was coloured from the same starting state is
        // still right, and so is everything after it.
        if (r > static_cast<std::size_t>(y) && row.hl_done && row.hl_in == in) {
            break;
        }
        row.hl_in = in;
        row.hl_done = true;
        if (e.highlight) {
            row.open_comment = highlight_row(row, in);
        } else {
            row.hl.assign(row.render.size(), Hl::Normal);
            row.open_comment = false;
        }
        in = row.open_comment;
    }
}

void refresh_row(Editor& e, int y) {
    update_render(e.rows[y]);
    e.rows[y].hl_done = false;
    update_syntax(e, y);
}

void select_syntax(Editor& e) {
    e.highlight = false;
    auto dot = e.filename.rfind('.');
    if (dot != std::string::npos) {
        std::string_view ext = std::string_view(e.filename).substr(dot);
        for (std::string_view c : {".c", ".h", ".cc", ".cpp", ".cxx", ".hh", ".hpp", ".hxx"}) {
            e.highlight = e.highlight || ext == c;
        }
    }
    for (Row& row : e.rows) {
        row.hl_done = false;
    }
    if (!e.rows.empty()) {
        update_syntax(e, 0);
    }
}

// The 16 palette colours, so kite follows your terminal's theme.
std::string_view hl_color(Hl hl) {
    switch (hl) {
    case Hl::Comment:
    case Hl::MlComment:
        return "36";  // cyan
    case Hl::Keyword:
        return "33";  // yellow
    case Hl::Type:
        return "32";  // green
    case Hl::String:
        return "35";  // magenta
    case Hl::Number:
        return "31";  // red
    case Hl::Preproc:
        return "34";  // blue
    case Hl::Match:
        return "7";  // inverted
    case Hl::Normal:
        break;
    }
    return "0";
}
