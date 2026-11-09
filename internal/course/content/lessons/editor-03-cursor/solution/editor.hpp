#pragma once
// The editor's state. You won't need to change this file.

#include <string>
#include <string_view>
#include <vector>

struct Row {
    std::string chars;   // the line's text
    std::string render;  // the line as it's drawn (the same, for now)
};

struct Editor {
    int cx = 0, cy = 0;  // the cursor: column and row in the text
    int screen_rows = 24;
    int screen_cols = 80;
    std::vector<Row> rows;
};

inline constexpr std::string_view kWelcome = "kite -- a tiny text editor";

// Moves the cursor for an arrow key, Home, End, Page Up or Page Down, and
// ignores any other key. The cursor stays on the text, or on the line just
// past its end.
void move_cursor(Editor& e, int key);

// Everything to write to the terminal to show the editor: the text, or
// tildes past its end, then the cursor in place.
std::string draw_screen(const Editor& e);
