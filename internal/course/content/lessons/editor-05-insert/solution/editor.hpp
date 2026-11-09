#pragma once
// The editor's state. You won't need to change this file.

#include <istream>
#include <string>
#include <string_view>
#include <vector>

inline constexpr int kTabStop = 8;

struct Row {
    std::string chars;   // the line's text
    std::string render;  // the line as it's drawn, with tabs turned into spaces
};

struct Editor {
    int cx = 0, cy = 0;          // the cursor: column and row in the text
    int rx = 0;                  // the cursor's column in render
    int rowoff = 0, coloff = 0;  // the first text row and render column on screen
    int screen_rows = 24;
    int screen_cols = 80;
    std::vector<Row> rows;
};

// The lines of a file, without their line endings ("\n" or "\r\n"), each
// with its render filled in.
std::vector<Row> read_rows(std::istream& in);

// Sets row.render from row.chars: each tab becomes spaces up to the next
// multiple of kTabStop columns.
void update_render(Row& row);

// The render column of chars column cx.
int cx_to_rx(const Row& row, int cx);

// Works out rx, then scrolls just enough to keep the cursor on screen.
void scroll(Editor& e);

inline constexpr std::string_view kWelcome = "kite -- a tiny text editor";

// Moves the cursor for an arrow key, Home, End, Page Up or Page Down, and
// ignores any other key. The cursor stays on the text, or on the line just
// past its end.
void move_cursor(Editor& e, int key);

// Types c at the cursor. On the line past the end, that line is added first.
void insert_char(Editor& e, char c);

// Enter: splits the line at the cursor, and moves to the start of the new
// line.
void insert_newline(Editor& e);

// Backspace: deletes the character before the cursor. At the start of a
// line, joins the line onto the one above.
void delete_char(Editor& e);

// Acts on one key. Returns false when it's time to quit.
bool process_key(Editor& e, int key);

// Everything to write to the terminal to show the editor: the part of the
// text that's scrolled into view, or tildes past its end, then the cursor.
std::string draw_screen(const Editor& e);
