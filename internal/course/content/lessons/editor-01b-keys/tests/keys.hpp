#pragma once
// decode_key, one part at a time. A special key can have any value above
// 255 that you like, so the tests compare keys with each other rather than
// with numbers: Home is the same key however the terminal spells it.
#include <string>
#include <string_view>
#include <vector>

#include "kite.hpp"

namespace ktest {

inline int key(std::string_view bytes) { return decode_key(bytes).first; }
inline std::size_t used(std::string_view bytes) { return decode_key(bytes).second; }

// One way to spell each key kite uses that isn't a byte.
inline const std::vector<std::string> kSpecialKeys = {
    "\x1b[A", "\x1b[B", "\x1b[C", "\x1b[D", "\x1b[H", "\x1b[F", "\x1b[3~", "\x1b[5~", "\x1b[6~",
};
inline const std::vector<std::string> kSpecialNames = {
    "Up", "Down", "Right", "Left", "Home", "End", "Delete", "Page Up", "Page Down",
};

}  // namespace ktest
