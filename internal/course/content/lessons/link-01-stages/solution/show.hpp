#pragma once
// Used by the SHOW macro in macros.hpp. You won't need to change this file.

#include <string>

// "text = value", for example show("1 + 2", 3) is "1 + 2 = 3".
inline std::string show(const char* text, long long value) {
    return std::string(text) + " = " + std::to_string(value);
}
