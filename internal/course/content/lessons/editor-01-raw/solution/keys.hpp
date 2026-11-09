#pragma once
// Keys, and turning the bytes a terminal sends into them. You won't need to
// change this file.

#include <cstddef>
#include <string_view>

// A key is a byte value (0-255) for ordinary keys, or one of these.
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
    kUnknown,  // an escape sequence this editor doesn't use, like F5
};

// Ctrl plus a letter sends the letter's code with the top three bits
// cleared: Ctrl-Q is 'q' & 0x1f, which is 17.
constexpr int ctrl_key(char c) {
    return c & 0x1f;
}

struct KeyPress {
    int key = 0;
    std::size_t used = 0;  // how many bytes of the input it took
};

// Decodes the first key in bytes, which holds at least one byte.
KeyPress decode_key(std::string_view bytes);
