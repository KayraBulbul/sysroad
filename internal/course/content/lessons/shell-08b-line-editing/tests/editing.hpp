#pragma once
// Typing at conch with the editing keys, and seeing what ran.

#include <string>

#include "conch_pty.hpp"

namespace shtest {

// What the keyboard sends for each key.
namespace keys {
inline const std::string up = "\x1b[A", down = "\x1b[B", right = "\x1b[C", left = "\x1b[D";
inline const std::string home = "\x1b[H", end = "\x1b[F", del = "\x1b[3~", backspace = "\x7f";
inline const std::string enter = "\r", ctrl_a = "\x01", ctrl_e = "\x05", ctrl_u = "\x15";
inline const std::string ctrl_k = "\x0b", ctrl_w = "\x17", ctrl_c = "\x03", ctrl_d = "\x04";
}  // namespace keys

inline std::string times(const std::string& key, int n) {
    std::string s;
    for (int i = 0; i < n; ++i) {
        s += key;
    }
    return s;
}

// Whether a command printed text on a line of its own, as `echo text`
// does: the editor never draws a newline right before what you typed.
inline bool printed(Conch& c, const std::string& text, int ms = 3000) {
    return c.expect("\n" + text + "\r", ms);
}

}  // namespace shtest
