// kite, for now: press keys and see what the terminal sends, and what
// decode_key makes of it. Ctrl-Q quits. You won't need to change this file.
#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <unistd.h>

#include "keys.hpp"
#include "terminal.hpp"

static std::string key_name(int key) {
    switch (key) {
    case kArrowLeft:
        return "ArrowLeft";
    case kArrowRight:
        return "ArrowRight";
    case kArrowUp:
        return "ArrowUp";
    case kArrowDown:
        return "ArrowDown";
    case kDelete:
        return "Delete";
    case kHome:
        return "Home";
    case kEnd:
        return "End";
    case kPageUp:
        return "PageUp";
    case kPageDown:
        return "PageDown";
    case kUnknown:
        return "(unknown sequence)";
    case kEnter:
        return "Enter";
    case kTab:
        return "Tab";
    case kEsc:
        return "Esc";
    case kBackspace:
        return "Backspace";
    }
    if (key < 32) {
        return std::format("Ctrl-{}", static_cast<char>(key + 'A' - 1));
    }
    if (key < 127) {
        return std::format("'{}'", static_cast<char>(key));
    }
    return std::format("byte {}", key);
}

int main() {
    try {
        RawMode raw(STDIN_FILENO);
        KeyReader keys(STDIN_FILENO);
        write_all("Press keys to see what the terminal sends. Ctrl-Q quits.\r\n");
        while (true) {
            int key = keys.next();
            std::string hex;
            for (unsigned char c : keys.last_bytes()) {
                hex += std::format(" {:02x}", c);
            }
            write_all(std::format("{:<20}{}\r\n", key_name(key), hex));
            if (key == ctrl_key('q')) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "kite: " << e.what() << '\n';
        return 1;
    }
}
