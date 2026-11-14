#pragma once
// kite as the tests drive it: your Editor in a window of the tests' size,
// a file opened, and each frame draw_screen returns shown on a Term.
#include <fstream>
#include <string>

#include "kite.hpp"
#include "term.hpp"

namespace ktest {

// Writes text to the file at path, in the folder the tests run in.
inline void write_file(const std::string& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << text;
}

struct Kite {
    Editor e;
    Term term;

    Kite(int rows, int cols) : term(rows, cols) { set_window_size(e, rows, cols); }

    // Writes text to a file called name, opens it, and draws.
    Kite& open(const std::string& name, const std::string& text) {
        write_file(name, text);
        open_file(e, name);
        return draw();
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
