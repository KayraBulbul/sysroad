#pragma once
// kite, a small text editor. The tests use what this header declares.

#include <cstddef>
#include <ctime>
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

// One line of the text.
struct Row {
    std::string chars;
};

struct Editor {
    std::vector<Row> rows;
    int cx = 0, cy = 0;    // the cursor: column and row in the text
    int screen_rows = 22;  // the rows for text: the window less the two bars
    int screen_cols = 80;
    std::string filename;         // empty for a file with no name yet
    std::string status_msg;       // the message under the status bar
    std::time_t status_time = 0;  // when it was set; it shows for 5 seconds
};

inline constexpr std::string_view kWelcome = "kite -- a tiny text editor";

// The terminal window is rows x cols.
void set_window_size(Editor& e, int rows, int cols);

// Loads the file at path, a Row per line. A file that isn't there opens
// empty, as a new file with that name.
void open_file(Editor& e, const std::string& path);

// Everything to write to the terminal to show the editor: the text, the
// status bar and the message bar.
std::string draw_screen(Editor& e);

// Shows msg in the message bar for 5 seconds.
void set_status(Editor& e, std::string msg);
