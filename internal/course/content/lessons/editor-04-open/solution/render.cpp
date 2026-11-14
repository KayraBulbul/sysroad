#include <format>

#include "kite.hpp"

namespace {

// The status bar's text, exactly screen_cols wide: the file's name and
// line count on the left, the cursor's line on the right.
std::string status_bar(const Editor& e) {
    auto width = static_cast<std::size_t>(e.screen_cols);
    std::string name = e.filename.empty() ? "[No Name]" : e.filename;
    std::string left = std::format("{:.20} - {} lines", name, e.rows.size());
    std::string right = std::format("{}/{}", e.cy + 1, e.rows.size());
    if (left.size() > width) {
        left.resize(width);
    }
    if (left.size() + right.size() <= width) {
        return left + std::string(width - left.size() - right.size(), ' ') + right;
    }
    return left + std::string(width - left.size(), ' ');
}

}  // namespace

std::string draw_screen(Editor& e) {
    scroll(e);
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
