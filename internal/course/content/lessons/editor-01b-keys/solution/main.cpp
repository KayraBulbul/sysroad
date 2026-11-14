// kite, for now: shows the name of each key you press, until Ctrl-Q.
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>

#include "kite.hpp"
#include "terminal.hpp"

static std::string key_name(int key) {
    switch (key) {
    case kArrowLeft:
        return "Left";
    case kArrowRight:
        return "Right";
    case kArrowUp:
        return "Up";
    case kArrowDown:
        return "Down";
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
        return "Unknown";
    case kEnter:
        return "Enter";
    case kTab:
        return "Tab";
    case kEsc:
        return "Esc";
    case kBackspace:
        return "Backspace";
    case ' ':
        return "Space";
    }
    if (key >= 1 && key <= 26) {
        return std::string("Ctrl-") + static_cast<char>('A' + key - 1);
    }
    if (key > 32 && key < 127) {
        return std::string(1, static_cast<char>(key));
    }
    return "byte " + std::to_string(key);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    try {
        RawMode raw(STDIN_FILENO);
        KeyReader keys(STDIN_FILENO);
        std::cout << "Press keys to see their names. Ctrl-Q quits.\r\n" << std::flush;
        while (true) {
            int key = keys.next();
            std::cout << key_name(key) << "\r\n" << std::flush;
            if (key == ctrl_key('q')) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "kite: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
