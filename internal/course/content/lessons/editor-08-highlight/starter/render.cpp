// The course's draw_screen, now in colour. You won't need to change this
// file.
#include <ctime>
#include <format>

#include "editor.hpp"

// One line of text in colour, from render column start, at most cols wide.
// Control characters are shown inverted, as @, A, B... (or ? for DEL and
// beyond), rather than sent to the terminal, which would act on them.
static void draw_row(std::string& out, const Row& row, std::size_t start, std::size_t cols) {
    std::string_view current = "0";
    for (std::size_t j = start; j < row.render.size() && j < start + cols; ++j) {
        auto c = static_cast<unsigned char>(row.render[j]);
        Hl hl = j < row.hl.size() ? row.hl[j] : Hl::Normal;
        std::string_view color = hl_color(hl);
        if (c < 32 || c == 127) {
            out += "\x1b[7m";
            out += c < 32 ? static_cast<char>('@' + c) : '?';
            out += std::format("\x1b[m\x1b[{}m", current);
            continue;
        }
        if (color != current) {
            out += std::format("\x1b[m\x1b[{}m", color);
            current = color;
        }
        out += static_cast<char>(c);
    }
    out += "\x1b[m";
}

std::string draw_screen(const Editor& e) {
    std::string out = "\x1b[?25l\x1b[H";  // hide the cursor, go to the top left
    auto cols = static_cast<std::size_t>(e.screen_cols);
    for (int y = 0; y < e.screen_rows; ++y) {
        auto filerow = static_cast<std::size_t>(y + e.rowoff);
        if (filerow < e.rows.size()) {
            draw_row(out, e.rows[filerow], static_cast<std::size_t>(e.coloff), cols);
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
    if (!e.status_msg.empty() && std::time(nullptr) - e.status_time < 5) {
        out += e.status_msg.substr(0, cols);
    }
    out += std::format("\x1b[{};{}H", e.cy - e.rowoff + 1, e.rx - e.coloff + 1);
    out += "\x1b[?25h";
    return out;
}
