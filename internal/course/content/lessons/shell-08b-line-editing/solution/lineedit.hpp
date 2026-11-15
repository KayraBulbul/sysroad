#pragma once
// The line being typed, apart from the terminal.

#include <cstdint>
#include <string>
#include <vector>

class LineEditor {
  public:
    enum Result { Editing, Done, Cancelled, EndOfInput };

    // history is oldest first; Up walks back through it from the newest.
    explicit LineEditor(const std::vector<std::string>& history) : history_(&history) {}

    // Handles one key: a printable character, Backspace, Delete, the
    // arrows, Home, End, Enter, or Ctrl-A, E, U, K, W, C, D.
    Result feed(int key);

    const std::string& text() const { return text_; }

    // What to write to redraw the line: back to column 0, the prompt and
    // the text, clear the rest, and the cursor where it belongs.
    std::string render(const std::string& prompt) const;

  private:
    const std::vector<std::string>* history_;
    std::string text_;
    size_t cursor_ = 0;
    size_t browsing_ = SIZE_MAX;  // the history entry shown; SIZE_MAX: the new line
    std::string draft_;           // the new line, while browsing history
};
