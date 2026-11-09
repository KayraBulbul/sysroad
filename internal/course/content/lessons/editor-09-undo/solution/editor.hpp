#pragma once
// The editor's state. You won't need to change this file.

#include <array>
#include <ctime>
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

inline constexpr int kTabStop = 8;
inline constexpr int kQuitTimes = 3;  // Ctrl-Qs to quit with unsaved changes

// A position in the text.
struct Pos {
    int row = 0;
    int col = 0;
    bool operator==(const Pos&) const = default;
};

// What each character on screen is, for its colour.
enum class Hl {
    Normal,
    Comment,    // // to the end of the line
    MlComment,  // /* ... */, which can span lines
    Keyword,    // if, return, class, ...
    Type,       // int, char, auto, ...
    String,     // "..." and '...'
    Number,     // 42, 3.14f, 0xff
    Preproc,    // #include and the rest of its line
    Match,      // the current search match
};

struct Row {
    std::string chars;          // the line's text
    std::string render;         // the line as it's drawn, with tabs turned into spaces
    std::vector<Hl> hl;         // one per character of render
    bool open_comment = false;  // the line ends inside a /* */ comment
    bool hl_in = false;         // whether it was highlighted starting inside one
    bool hl_done = false;       // hl is up to date
};

inline constexpr std::array<std::string_view, 37> kKeywords = {
    "if",        "else",     "for",     "while",     "do",      "switch",    "case",   "default",
    "break",     "continue", "return",  "goto",      "try",     "catch",     "throw",  "new",
    "delete",    "class",    "struct",  "union",     "enum",    "namespace", "using",  "template",
    "typename",  "public",   "private", "protected", "virtual", "override",  "static", "const",
    "constexpr", "inline",   "nullptr", "true",      "false",
};

inline constexpr std::array<std::string_view, 12> kTypes = {
    "int",    "long", "short", "char",     "bool",   "float",
    "double", "void", "auto",  "unsigned", "signed", "size_t",
};

// One change to the text: text inserted at `at`, or erased from there. The
// text can hold '\n's, which split or join lines.
struct Edit {
    enum class Kind { Insert, Erase };
    Kind kind = Kind::Insert;
    Pos at;
    std::string text;
    Pos cursor;  // where the cursor was just before the change
};

// What undo and redo can do.
struct History {
    std::vector<Edit> undo;  // the newest last
    std::vector<Edit> redo;  // what undo took back, the newest last
    bool sealed = false;     // the next edit starts a new step, even if it could join the last
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
    bool highlight = false;  // the file is C or C++, so it gets colours
    History history;
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

// Whether c ends a word: whitespace, the NUL byte, or one of
// ,.()+-/*=~%<>[]{};:!&|^?
bool is_separator(char c);

// Sets row.hl, one entry per character of row.render, for C++ code. in_comment
// says whether the row starts inside a /* */ comment. Returns whether it ends
// inside one.
bool highlight_row(Row& row, bool in_comment);

// Re-highlights row y, and the rows after it for as long as a /* */ comment
// opening or closing changes how they start.
void update_syntax(Editor& e, int y);

// After row y's chars change: its render, then its colours.
void refresh_row(Editor& e, int y);

// Turns highlighting on for C and C++ files, by their name, and colours the
// whole file.
void select_syntax(Editor& e);

// The terminal colour code for a kind of text.
std::string_view hl_color(Hl hl);

// The status bar's text, exactly screen_cols wide: the file's name (at most
// 20 characters) and line count, "(modified)" when it's dirty, and the
// cursor's line on the right.
std::string status_bar(const Editor& e);

// Finds query in the text, scanning from `from` towards the end (dir = 1)
// or the start (dir = -1), and wrapping around. A match at `from` itself
// counts. Returns where the match starts, or nothing if there's none
// anywhere (or the query is empty). Matches don't cross lines.
std::optional<Pos> find(const std::vector<Row>& rows, std::string_view query, Pos from, int dir);

// Shows msg in the message bar for 5 seconds.
void set_status(Editor& e, std::string msg);

// Loads a file, or starts a new one with that name if it doesn't exist.
void open_file(Editor& e, const std::string& path);

// Inserts text at `at` (on the line past the end, that line is added first)
// and returns the position just after it. Rows are kept up to date.
Pos insert_text(Editor& e, Pos at, std::string_view text);

// Erases n characters from `at`, where each line break counts as one.
void erase_text(Editor& e, Pos at, std::size_t n);

// Delete: erases the character under the cursor, or joins the next line on.
void delete_forward(Editor& e);

// Adds an edit to the undo history and empties redo. An edit that carries
// on from the last one joins its step, unless the history is sealed: typing
// joins typing, except a word after a space; Backspace joins Backspace;
// Delete joins Delete. Nothing with a line break joins. Recording unseals.
void record(History& h, Edit edit);

// Takes back the newest step: reverses it, puts the cursor where it was
// before, and moves the step to redo. Returns false if there's nothing to
// undo.
bool undo(Editor& e);

// Does the newest undone step again, with the cursor after it. Returns false
// if there's nothing to redo.
bool redo(Editor& e);

// Acts on one key. Returns false when it's time to quit. Keys reads more
// keys when a prompt needs them.
bool process_key(Editor& e, KeyReader& keys, int key);

// Everything to write to the terminal to show the editor: the part of the
// text that's scrolled into view, the status bar and the message bar, then
// the cursor.
std::string draw_screen(const Editor& e);
