#include "lineedit.hpp"

#include "keys.hpp"

LineEditor::Result LineEditor::feed(int key) {
    // TODO: every key the header lists. For now: type, and Enter.
    if (key == kEnter || key == '\n') {
        return Done;
    }
    if (key >= 32 && key < 127) {
        text_ += static_cast<char>(key);
        cursor_ = text_.size();
    }
    return Editing;
}

std::string LineEditor::render(const std::string& prompt) const {
    // TODO: clear the rest of the line, and put the cursor in place.
    return "\r" + prompt + text_;
}
