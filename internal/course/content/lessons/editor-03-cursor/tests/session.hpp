#pragma once
// kite as the tests drive it: your Editor in a window of the tests' size,
// a file opened, keys pressed one at a time with a frame drawn after each,
// as main does, and each frame draw_screen returns shown on a Term.
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

#include "kite.hpp"
#include "term.hpp"

namespace ktest {

// Writes text to the file at path, in the folder the tests run in.
inline void write_file(const std::string& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << text;
}

// What the keyboard sends for each key.
namespace keys {
inline constexpr std::string_view up = "\x1b[A", down = "\x1b[B", right = "\x1b[C", left = "\x1b[D";
inline constexpr std::string_view home = "\x1b[H", end = "\x1b[F", page_up = "\x1b[5~", page_down = "\x1b[6~";
inline constexpr std::string_view del = "\x1b[3~", backspace = "\x7f", enter = "\r", esc = "\x1b", tab = "\t";
inline constexpr std::string_view ctrl_q = "\x11", ctrl_s = "\x13", ctrl_f = "\x06", ctrl_c = "\x03";
inline constexpr std::string_view ctrl_z = "\x1a", ctrl_y = "\x19", f5 = "\x1b[15~";
}  // namespace keys

// Reads a file back.
inline std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}

struct Kite {
    Editor e;
    Term term;
    bool quit = false;  // process_key said it's time to quit

    Kite(int rows, int cols) : term(rows, cols) { set_window_size(e, rows, cols); }

    // Writes text to a file called name, opens it, and draws.
    Kite& open(const std::string& name, const std::string& text) {
        write_file(name, text);
        open_file(e, name);
        return draw();
    }

    // Presses the keys in bytes, decoded with your decode_key, `times`
    // times over, drawing a frame after each key.
    Kite& press(std::string_view bytes, int times = 1) {
        for (int i = 0; i < times; ++i) {
            std::string_view rest = bytes;
            while (!rest.empty()) {
                auto [key, used] = decode_key(rest);
                quit = !process_key(e, key);
                draw();
                rest.remove_prefix(std::clamp<std::size_t>(used, 1, rest.size()));
            }
        }
        return *this;
    }

    // Types text, a key per character, with Enter for each "\n".
    Kite& type(std::string_view text) {
        for (char c : text) {
            press(c == '\n' ? keys::enter : std::string_view(&c, 1));
        }
        return *this;
    }

    // Draws a frame.
    Kite& draw() {
        term.write(draw_screen(e));
        return *this;
    }

    // What's on the screen, a line per row, without trailing spaces.
    std::string screen() const { return term.screen(); }
    // Rows first to last-1 of it.
    std::string rows(int first, int last) const { return term.lines(first, last); }
    std::string line(int r) const { return term.line(r); }
    // Row r's cells: '#' where they're inverted (\x1b[7m), '.' where not.
    std::string inverted(int r) const {
        std::string s;
        for (int c = 0; c < term.cols(); ++c) {
            s += term.at(r, c).inverse ? '#' : '.';
        }
        return s;
    }
    // Where the cursor is, "row R, col C", counting from 0.
    std::string cursor() const { return term.cursor(); }
};

}  // namespace ktest
