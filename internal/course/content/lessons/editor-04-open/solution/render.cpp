// The course's version of what you wrote, now drawing the part of the text
// that's scrolled into view. You won't need to change this file.
#include <format>

#include "editor.hpp"

std::string draw_screen(const Editor& e) {
    std::string out = "\x1b[?25l\x1b[H";  // hide the cursor, go to the top left
    auto cols = static_cast<std::size_t>(e.screen_cols);
    for (int y = 0; y < e.screen_rows; ++y) {
        auto filerow = static_cast<std::size_t>(y + e.rowoff);
        if (filerow < e.rows.size()) {
            const std::string& line = e.rows[filerow].render;
            if (static_cast<std::size_t>(e.coloff) < line.size()) {
                out += line.substr(static_cast<std::size_t>(e.coloff), cols);
            }
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
        out += "\x1b[K";  // clear the rest of the line
        if (y < e.screen_rows - 1) {
            out += "\r\n";
        }
    }
    out += std::format("\x1b[{};{}H", e.cy - e.rowoff + 1, e.rx - e.coloff + 1);
    out += "\x1b[?25h";
    return out;
}
