#pragma once
// Keys, and turning the bytes a terminal sends into them: kite's decoder,
// brought over.

#include <cstddef>
#include <string_view>
#include <utility>

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
    kUnknown,  // an escape sequence conch doesn't use, like F5
};

// Ctrl plus a letter: the letter's code with the top three bits cleared.
constexpr int ctrl_key(char c) {
    return c & 0x1f;
}

// The first key in bytes, and how many bytes it took.
std::pair<int, std::size_t> decode_key(std::string_view bytes);
