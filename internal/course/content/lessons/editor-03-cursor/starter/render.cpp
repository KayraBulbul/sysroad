// The course's version of what you wrote last lesson. You won't need to
// change this file.
#include <format>

#include "editor.hpp"

std::string draw_screen(const Editor& e) {
    std::string out = "\x1b[?25l\x1b[H";  // hide the cursor, go to the top left
    auto cols = static_cast<std::size_t>(e.screen_cols);
    for (int y = 0; y < e.screen_rows; ++y) {
        if (static_cast<std::size_t>(y) < e.rows.size()) {
            out += e.rows[y].render.substr(0, cols);
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
    out += std::format("\x1b[{};{}H", e.cy + 1, e.cx + 1);
    out += "\x1b[?25h";
    return out;
}
