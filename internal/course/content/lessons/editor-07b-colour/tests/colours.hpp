#pragma once
// Reads colours off the screen as one letter per character, so a test can
// line them up under the text:
//
//   n  normal: the terminal's own colour (\x1b[39m, or \x1b[m)
//   d  red (31), numbers            s  magenta (35), strings
//   k  yellow (33), keywords        t  green (32), types
//   c  cyan (36), comments          p  blue (34), the preprocessor
//   #  inverted (\x1b[7m), whatever the colour
//   ?  any other colour
//
// The same colour from the 256-colour palette (\x1b[38;5;1m) counts the
// same; bold doesn't matter.
#include <string>

#include "session.hpp"

namespace ktest {

inline char colour_letter(const Term::Cell& cell) {
    if (cell.inverse) {
        return '#';
    }
    switch (cell.fg) {
    case -1:
        return 'n';
    case 1:
        return 'd';
    case 5:
        return 's';
    case 3:
        return 'k';
    case 2:
        return 't';
    case 6:
        return 'c';
    case 4:
        return 'p';
    default:
        return '?';
    }
}

// The colours of screen row r's first n characters.
inline std::string colours(const Kite& k, int r, std::size_t n) {
    std::string out;
    for (int c = 0; c < static_cast<int>(n) && c < k.term.cols(); ++c) {
        out += colour_letter(k.term.at(r, c));
    }
    return out;
}

// How wide a line is on screen, with its tabs turned into spaces.
inline std::size_t screen_width(const std::string& line) {
    std::size_t w = 0;
    for (char c : line) {
        w = c == '\t' ? (w / 8 + 1) * 8 : w + 1;
    }
    return w;
}

// The colours kite gives a file called name holding just this line.
inline std::string hl(const std::string& line, const std::string& name = "hl.cpp") {
    Kite k(6, 100);
    k.open(name, line + "\n");
    return colours(k, 0, screen_width(line));
}

}  // namespace ktest
