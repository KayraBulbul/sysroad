#pragma once
// kite, a small text editor. The tests use what this header declares.

#include <cstddef>
#include <ctime>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <termios.h>
#include <utility>
#include <vector>

// The settings for raw mode, made from the terminal's current ones.
termios raw_attrs(termios t);

// A key is a byte value (0-255) for an ordinary key, or one of these.
enum Key : int {
    kTab = '\t',
    kEnter = '\r',
    kEsc = 27,
    kBackspace = 127,
    kArrowLeft = 1000,
    kArrowRight,
    kArrowUp,
    kArrowDown,
    kDelete,
    kHome,
    kEnd,
    kPageUp,
    kPageDown,
    kUnknown,  // an escape sequence kite doesn't use, like F5
};

// Ctrl plus a letter sends the letter's code with the top three bits
// cleared: Ctrl-Q is 'q' & 0x1f, which is 17.
constexpr int ctrl_key(char c) {
    return c & 0x1f;
}

// The first key in bytes, and how many bytes it took. bytes holds at
// least one byte.
std::pair<int, std::size_t> decode_key(std::string_view bytes);

inline constexpr int kTabStop = 8;
inline constexpr int kQuitTimes = 3;  // more Ctrl-Qs to quit with unsaved changes

// What each character on screen is, for its colour.
enum class Hl {
    Normal,
    Number,  // 42, 3.14f, 0xff
    String,  // "..." and '...'
};

// One line of the text.
struct Row {
    std::string chars;   // the line as it is in the file
    std::string render;  // the line as it's drawn, with tabs turned into spaces
    std::vector<Hl> hl;  // one per character of render
};

struct Editor;

// A question in the message bar, like "Save as: ". Keys go to it until
// Enter or Escape, then done gets the answer, or nothing for Escape.
// on_key, if set, hears about every other key, after the answer has
// taken it.
struct Prompt {
    std::string label;
    std::string answer;
    std::function<void(Editor&, std::optional<std::string>)> done;
    std::function<void(Editor&, const std::string& answer, int key)> on_key;
};

// A place in the text.
struct Pos {
    int row = 0;
    int col = 0;
};

struct Editor {
    std::vector<Row> rows;
    int cx = 0, cy = 0;          // the cursor: column and row in the text
    int rx = 0;                  // the cursor's column in render
    int rowoff = 0, coloff = 0;  // the first text row and render column on screen
    int screen_rows = 22;  // the rows for text: the window less the two bars
    int screen_cols = 80;
    std::string filename;         // empty for a file with no name yet
    std::string status_msg;       // the message under the status bar
    std::time_t status_time = 0;  // when it was set; it shows for 5 seconds
    bool dirty = false;           // changed since it was opened or saved
    int quit_times = kQuitTimes;
    std::optional<Prompt> prompt;  // the question being asked, if any
    bool highlight = false;        // the file is C or C++, so it gets colours
    std::optional<Pos> match;      // the search's match, shown inverted
    int match_len = 0;
};

inline constexpr std::string_view kWelcome = "kite -- a tiny text editor";

// The terminal window is rows x cols.
void set_window_size(Editor& e, int rows, int cols);

// Loads the file at path, a Row per line, without "\n" or "\r\n". A file that isn't there opens
// empty, as a new file with that name.
void open_file(Editor& e, const std::string& path);

// Everything to write to the terminal to show the editor: the text, the
// status bar and the message bar.
std::string draw_screen(Editor& e);

// Sets row.render from row.chars: each tab becomes spaces up to the next
// multiple of kTabStop columns.
void update_render(Row& row);

// The render column of chars column cx.
int cx_to_rx(const Row& row, int cx);

// Works out rx, then scrolls just enough to keep the cursor on screen.
void scroll(Editor& e);

// Types c at the cursor. On the line past the end, that line is added first.
void insert_char(Editor& e, char c);

// Enter: splits the line at the cursor, and moves to the start of the new
// line.
void insert_newline(Editor& e);

// Backspace: deletes the character before the cursor. At the start of a
// line, joins the line onto the one above.
void delete_char(Editor& e);

// Delete: deletes the character at the cursor. At the end of a line, joins
// the next line onto it.
void delete_forward(Editor& e);

// The file as text: every row followed by "\n".
std::string rows_to_string(const std::vector<Row>& rows);

// Writes the text to e.filename, through a temporary file renamed over it,
// then clears dirty and says how many bytes it wrote. If it can't, the
// message starts "Can't save!", dirty stays set, and it returns false.
bool save_file(Editor& e);

// Finds query in the rows, scanning from `from` towards the end (dir 1)
// or the start (dir -1), and wrapping around. A match at `from` itself
// counts. Returns where the match starts, or nothing if there's none
// anywhere (or the query is empty). Matches don't cross lines.
std::optional<Pos> find(const std::vector<Row>& rows, std::string_view query, Pos from, int dir);

// Ctrl-F: search as you type.
void start_search(Editor& e);

// Whether c ends a word: whitespace, the NUL byte, or one of
// ,.()+-/*=~%<>[]{};:!&|^?
bool is_separator(char c);

// Sets row.hl, one entry per character of row.render, for C++ code.
void highlight_row(Row& row);

// After row y's chars change: its render, then its colours.
void refresh_row(Editor& e, int y);

// Turns colours on for C and C++ files, by their name, and colours every row.
void select_syntax(Editor& e);

// Acts on one key. Returns false when it's time to quit.
bool process_key(Editor& e, int key);

// Shows msg in the message bar for 5 seconds.
void set_status(Editor& e, std::string msg);
