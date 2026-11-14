#pragma once
// kite, a small text editor. The tests use what this header declares.

#include <cstddef>
#include <string_view>
#include <termios.h>
#include <utility>

// The settings for raw mode, made from the terminal's current ones.
termios raw_attrs(termios t);

// A key is a byte value (0-255) for an ordinary key, or one of these.
enum Key : int {
    kTab = '\t',
    kEnter = '\r',
    kEsc = 27,
    kBackspace = 127,
    kArrowLeft = 1000,
    kArrowRight,
    kArrowUp,
    kArrowDown,
    kDelete,
    kHome,
    kEnd,
    kPageUp,
    kPageDown,
    kUnknown,  // an escape sequence kite doesn't use, like F5
};

// Ctrl plus a letter sends the letter's code with the top three bits
// cleared: Ctrl-Q is 'q' & 0x1f, which is 17.
constexpr int ctrl_key(char c) {
    return c & 0x1f;
}

// The first key in bytes, and how many bytes it took. bytes holds at
// least one byte.
std::pair<int, std::size_t> decode_key(std::string_view bytes);
