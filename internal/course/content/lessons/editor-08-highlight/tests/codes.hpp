#pragma once
// Shows a row's colours as one letter per character, so a test can line
// them up under the text:
//   n normal, c comment, m /* comment */, k keyword, t type, s string,
//   d number, p preprocessor
#include <string>

#include "editor.hpp"

inline std::string codes(const Row& row) {
    std::string out;
    for (Hl h : row.hl) {
        switch (h) {
        case Hl::Normal:
            out += 'n';
            break;
        case Hl::Comment:
            out += 'c';
            break;
        case Hl::MlComment:
            out += 'm';
            break;
        case Hl::Keyword:
            out += 'k';
            break;
        case Hl::Type:
            out += 't';
            break;
        case Hl::String:
            out += 's';
            break;
        case Hl::Number:
            out += 'd';
            break;
        case Hl::Preproc:
            out += 'p';
            break;
        case Hl::Match:
            out += 'x';
            break;
        }
    }
    return out;
}

inline Row row(const std::string& text) {
    Row r{text, ""};
    update_render(r);
    return r;
}

// Highlights text as a whole line that starts outside any comment.
inline std::string hl(const std::string& text, bool in_comment = false) {
    Row r = row(text);
    highlight_row(r, in_comment);
    return codes(r);
}
