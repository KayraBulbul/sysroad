// The course's version of kite's key decoder (phase 3). You won't need to
// change this file.
#include "keys.hpp"

namespace {

// ESC [ <parameters> <final byte>: the parameters are bytes 0x30-0x3f
// (digits and ;), and the final byte is 0x40-0x7e (a letter, or ~).
KeyPress decode_csi(std::string_view bytes) {
    std::size_t i = 2;
    while (i < bytes.size() && bytes[i] >= 0x30 && bytes[i] <= 0x3f) {
        ++i;
    }
    if (i == bytes.size() || bytes[i] < 0x40 || bytes[i] > 0x7e) {
        return {kEsc, 1};  // not a whole sequence: just Escape
    }
    std::string_view params = bytes.substr(2, i - 2);
    char final = bytes[i];
    std::size_t used = i + 1;
    if (params.empty()) {
        switch (final) {
        case 'A':
            return {kArrowUp, used};
        case 'B':
            return {kArrowDown, used};
        case 'C':
            return {kArrowRight, used};
        case 'D':
            return {kArrowLeft, used};
        case 'H':
            return {kHome, used};
        case 'F':
            return {kEnd, used};
        }
    }
    if (final == '~' && params.size() == 1) {
        switch (params[0]) {
        case '1':
        case '7':
            return {kHome, used};
        case '3':
            return {kDelete, used};
        case '4':
        case '8':
            return {kEnd, used};
        case '5':
            return {kPageUp, used};
        case '6':
            return {kPageDown, used};
        }
    }
    return {kUnknown, used};
}

}  // namespace

KeyPress decode_key(std::string_view bytes) {
    if (bytes.empty()) {
        return {};
    }
    if (bytes[0] != '\x1b' || bytes.size() == 1) {
        return {static_cast<unsigned char>(bytes[0]), 1};
    }
    if (bytes[1] == '[') {
        return decode_csi(bytes);
    }
    if (bytes[1] == 'O' && bytes.size() >= 3) {
        switch (bytes[2]) {
        case 'H':
            return {kHome, 3};
        case 'F':
            return {kEnd, 3};
        }
        return {kUnknown, 3};
    }
    return {kEsc, 1};
}
