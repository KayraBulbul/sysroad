#include <format>

#include "kite.hpp"

namespace {

// The status bar's text, exactly screen_cols wide: the file's name and
// line count on the left, the cursor's line on the right.
std::string status_bar(const Editor& e) {
    auto width = static_cast<std::size_t>(e.screen_cols);
    std::string name = e.filename.empty() ? "[No Name]" : e.filename;
    std::string left =
        std::format("{:.20} - {} lines{}", name, e.rows.size(), e.dirty ? " (modified)" : "");
    std::string right = std::format("{}/{}", e.cy + 1, e.rows.size());
    if (left.size() > width) {
        left.resize(width);
    }
    if (left.size() + right.size() <= width) {
        return left + std::string(width - left.size() - right.size(), ' ') + right;
    }
    return left + std::string(width - left.size(), ' ');
}

// The terminal colour for a kind of text: the 16 palette colours, so kite
// follows your terminal's theme.
std::string_view hl_color(Hl hl) {
    switch (hl) {
    case Hl::Number:
        return "31";  // red
    case Hl::String:
        return "35";  // magenta
    case Hl::Comment:
    case Hl::MlComment:
        return "36";  // cyan
    case Hl::Keyword:
        return "33";  // yellow
    case Hl::Type:
        return "32";  // green
    case Hl::Preproc:
        return "34";  // blue
    case Hl::Normal:
        break;
    }
    return "39";  // the terminal's own
}

// Text row y, in colour, from render column start, at most cols wide.
// Control characters are shown inverted, as @, A, B... (or ? for DEL),
// rather than sent to the terminal, which would act on them. The search's
// match is inverted too.
void draw_row(std::string& out, const Editor& e, int y, std::size_t start, std::size_t cols) {
    const Row& row = e.rows[y];
    std::size_t match_from = 0, match_to = 0;
    if (e.match && e.match->row == y) {
        match_from = static_cast<std::size_t>(cx_to_rx(row, e.match->col));
        match_to = static_cast<std::size_t>(cx_to_rx(row, e.match->col + e.match_len));
    }
    std::string_view color = "39";
    bool inverse = false;
    for (std::size_t j = start; j < row.render.size() && j < start + cols; ++j) {
        auto c = static_cast<unsigned char>(row.render[j]);
        bool control = c < 32 || c == 127;
        std::string_view want = hl_color(j < row.hl.size() ? row.hl[j] : Hl::Normal);
        bool want_inverse = control || (j >= match_from && j < match_to);
        if (want != color || want_inverse != inverse) {
            out += std::format("\x1b[{};{}m", want, want_inverse ? "7" : "27");
            color = want;
            inverse = want_inverse;
        }
        if (control) {
            out += c < 32 ? static_cast<char>('@' + c) : '?';
        } else {
            out += static_cast<char>(c);
        }
    }
    out += "\x1b[m";
}

}  // namespace

std::string draw_screen(Editor& e) {
    scroll(e);
    std::string out = "\x1b[?25l\x1b[H";  // hide the cursor, go to the top left
    auto cols = static_cast<std::size_t>(e.screen_cols);
    for (int y = 0; y < e.screen_rows; ++y) {
        auto filerow = static_cast<std::size_t>(y + e.rowoff);
        if (filerow < e.rows.size()) {
            draw_row(out, e, static_cast<int>(filerow), static_cast<std::size_t>(e.coloff), cols);
        } else if (e.rows.empty() && y == e.screen_rows / 3) {
            std::string_view welcome = kWelcome.substr(0, cols);
            std::size_t padding = (cols - welcome.size()) / 2;
            if (padding > 0) {
                out += '~';
                out.append(padding - 1, ' ');
            }
            out += welcome;
        } else {
            out += '~';
        }
        out += "\x1b[K\r\n";  // clear the rest of the line
    }
    out += "\x1b[7m" + status_bar(e) + "\x1b[m\r\n";  // inverted colours
    out += "\x1b[K";
    if (e.prompt) {
        out += (e.prompt->label + e.prompt->answer).substr(0, cols);
    } else if (!e.status_msg.empty() && std::time(nullptr) - e.status_time < 5) {
        out += e.status_msg.substr(0, cols);
    }
    out += std::format("\x1b[{};{}H", e.cy - e.rowoff + 1, e.rx - e.coloff + 1);
    out += "\x1b[?25h";
    return out;
}
