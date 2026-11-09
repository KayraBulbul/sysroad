#include "keys.hpp"

#include <stdexcept>

KeyPress decode_key(std::string_view bytes) {
    // TODO: an ordinary byte is a key by itself. ESC [ ... and ESC O ...
    // are escape sequences: see the lesson for the ones to know.
    (void)bytes;
    throw std::logic_error("decode_key isn't written yet");
}
