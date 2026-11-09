#pragma once
// The editor's state. You won't need to change this file.

#include <ctime>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

inline constexpr int kTabStop = 8;
inline constexpr int kQuitTimes = 3;  // Ctrl-Qs to quit with unsaved changes

struct Row {
    std::string chars;   // the line's text
    std::string render;  // the line as it's drawn, with tabs turned into spaces
};

struct Editor {
    int cx = 0, cy = 0;          // the cursor: column and row in the text
    int rx = 0;                  // the cursor's column in render
    int rowoff = 0, coloff = 0;  // the first text row and render column on screen
    int screen_rows = 24;        // the text's rows: the window less the two bars
    int screen_cols = 80;
    std::vector<Row> rows;
    std::string filename;         // empty until it's saved for the first time
    bool dirty = false;           // changed since it was opened or saved
    std::string status_msg;       // the message under the status bar
    std::time_t status_time = 0;  // when it was set; it shows for 5 seconds
    int quit_times = kQuitTimes;
};

class KeyReader;

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

// The file as text: every row followed by "\n".
std::string rows_to_string(const std::vector<Row>& rows);

// Writes the text to e.filename, then clears dirty and says how many bytes
// it wrote. If it can't, the message starts "Can't save!", dirty stays set,
// and it returns false.
bool save_file(Editor& e);

// The status bar's text, exactly screen_cols wide: the file's name (at most
// 20 characters) and line count, "(modified)" when it's dirty, and the
// cursor's line on the right.
std::string status_bar(const Editor& e);

// Shows msg in the message bar for 5 seconds.
void set_status(Editor& e, std::string msg);

// Loads a file, or starts a new one with that name if it doesn't exist.
void open_file(Editor& e, const std::string& path);

// Acts on one key. Returns false when it's time to quit. Keys reads more
// keys when a prompt needs them.
bool process_key(Editor& e, KeyReader& keys, int key);

// Everything to write to the terminal to show the editor: the part of the
// text that's scrolled into view, the status bar and the message bar, then
// the cursor.
std::string draw_screen(const Editor& e);
