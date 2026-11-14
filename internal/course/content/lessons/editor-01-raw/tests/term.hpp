#pragma once
// A small terminal for the tests. It reads the bytes a program writes, the
// way your terminal does, and keeps what they leave on the screen: a grid
// of characters with their colours, and the cursor. So the tests look at
// what kite shows, not at which escape codes it chose to show it with:
// \x1b[H and \x1b[1;1H, or \x1b[m and \x1b[0m, come out the same.
//
// It understands the VT100/xterm codes a program like kite might use:
// cursor moves (CUP, CUU/CUD/CUF/CUB, CHA, VPA, ...), erasing (ED, EL,
// ECH), inserting and deleting characters and lines, SGR colours (16, 256
// and 24-bit), showing and hiding the cursor, saving and restoring it, and
// the alternate screen. Other codes are read and ignored. Writing past the
// bottom line scrolls, and writing in the last column wraps on the next
// character, as xterm does. One forgiving choice: erasing right after a
// character was written in the last column leaves that character alone,
// so a full-width line followed by \x1b[K keeps its last character.
// (Terminals differ there.)

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace ktest {

// Whether part is somewhere in s.
inline bool contains(const std::string& s, const std::string& part) { return s.find(part) != std::string::npos; }

class Term {
  public:
    // A colour: -1 is the terminal's default, 0-7 the normal palette
    // (black, red, green, yellow, blue, magenta, cyan, white), 8-15 the
    // bright one, 16-255 the rest of the 256, and 0x1000000 | rgb for
    // 24-bit colour.
    struct Cell {
        char ch = ' ';
        int fg = -1;
        int bg = -1;
        bool inverse = false;
        bool bold = false;
    };

    Term(int rows, int cols) : rows_(rows), cols_(cols), main_(blank_grid()), alt_(blank_grid()) {}

    // What a program writes to the terminal.
    void write(std::string_view bytes) {
        for (char c : bytes) {
            step(static_cast<unsigned char>(c));
        }
    }

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    const Cell& at(int row, int col) const { return grid()[static_cast<std::size_t>(row * cols_ + col)]; }

    // Row r's characters, without the spaces at its end.
    std::string line(int r) const {
        std::string s;
        for (int c = 0; c < cols_; ++c) {
            s += at(r, c).ch;
        }
        s.erase(s.find_last_not_of(' ') + 1);
        return s;
    }

    // Every row, each followed by "\n", without the spaces at their ends.
    std::string screen() const {
        std::string s;
        for (int r = 0; r < rows_; ++r) {
            s += line(r) + "\n";
        }
        return s;
    }

    // Rows first to last-1.
    std::string lines(int first, int last) const {
        std::string s;
        for (int r = first; r < last && r < rows_; ++r) {
            s += line(r) + "\n";
        }
        return s;
    }

    int cursor_row() const { return row_; }
    int cursor_col() const { return col_; }
    bool cursor_visible() const { return cursor_visible_; }
    bool on_alt_screen() const { return on_alt_; }

    // Where the cursor is, as "row R, col C", counting from 0.
    std::string cursor() const { return "row " + std::to_string(row_) + ", col " + std::to_string(col_); }

    // What the terminal has to say back, such as the answer to a cursor
    // position query (\x1b[6n). Taken once.
    std::string take_replies() {
        std::string r;
        r.swap(replies_);
        return r;
    }

  private:
    enum class State { Ground, Esc, EscSkip, Csi, String, StringEsc };

    std::vector<Cell> blank_grid() const {
        return std::vector<Cell>(static_cast<std::size_t>(rows_ * cols_));
    }
    std::vector<Cell>& grid() { return on_alt_ ? alt_ : main_; }
    const std::vector<Cell>& grid() const { return on_alt_ ? alt_ : main_; }
    Cell& cell(int row, int col) { return grid()[static_cast<std::size_t>(row * cols_ + col)]; }

    // An erased cell keeps the current background colour, as most
    // terminals do, but nothing else.
    Cell erased() const {
        Cell c;
        c.bg = pen_.bg;
        return c;
    }

    void step(unsigned char c) {
        switch (state_) {
        case State::Ground:
            ground(c);
            break;
        case State::Esc:
            escape(c);
            break;
        case State::EscSkip:
            state_ = State::Ground;  // the one byte after ESC ( and friends
            break;
        case State::Csi:
            csi_byte(c);
            break;
        case State::String:  // OSC, DCS and the like, up to BEL or ESC backslash
            if (c == 0x07) {
                state_ = State::Ground;
            } else if (c == 0x1b) {
                state_ = State::StringEsc;
            }
            break;
        case State::StringEsc:
            state_ = c == '\\' ? State::Ground : State::String;
            break;
        }
    }

    void ground(unsigned char c) {
        if (c == 0x1b) {
            state_ = State::Esc;
            return;
        }
        if (c < 0x20 || c == 0x7f) {
            control(c);
            return;
        }
        print(static_cast<char>(c));
    }

    void control(unsigned char c) {
        switch (c) {
        case '\r':
            col_ = 0;
            wrap_ = false;
            break;
        case '\n':
        case '\v':
        case '\f':
            line_feed();
            break;
        case '\b':
            if (col_ > 0) {
                --col_;
            }
            wrap_ = false;
            break;
        case '\t':
            col_ = std::min(cols_ - 1, (col_ / 8 + 1) * 8);
            wrap_ = false;
            break;
        default:
            break;  // BEL, NUL, DEL and the rest show nothing
        }
    }

    void print(char ch) {
        if (wrap_) {
            col_ = 0;
            line_feed();
        }
        Cell& cl = cell(row_, col_);
        cl = pen_;
        cl.ch = ch;
        if (col_ == cols_ - 1) {
            wrap_ = true;
        } else {
            ++col_;
        }
    }

    void line_feed() {
        wrap_ = false;
        if (row_ == rows_ - 1) {
            scroll_up(1);
        } else {
            ++row_;
        }
    }

    void scroll_up(int n) {
        auto& g = grid();
        for (int i = 0; i < n; ++i) {
            g.erase(g.begin(), g.begin() + cols_);
            g.insert(g.end(), static_cast<std::size_t>(cols_), erased());
        }
    }

    void scroll_down(int n) {
        auto& g = grid();
        for (int i = 0; i < n; ++i) {
            g.erase(g.end() - cols_, g.end());
            g.insert(g.begin(), static_cast<std::size_t>(cols_), erased());
        }
    }

    void escape(unsigned char c) {
        state_ = State::Ground;
        switch (c) {
        case '[':
            state_ = State::Csi;
            params_.clear();
            private_ = 0;
            intermediate_ = false;
            break;
        case ']':
        case 'P':
        case 'X':
        case '^':
        case '_':
            state_ = State::String;
            break;
        case '(':
        case ')':
        case '*':
        case '+':
        case '#':
        case '%':
            state_ = State::EscSkip;
            break;
        case '7':
            save_cursor();
            break;
        case '8':
            restore_cursor();
            break;
        case 'D':
            line_feed();
            break;
        case 'E':
            col_ = 0;
            line_feed();
            break;
        case 'M':
            wrap_ = false;
            if (row_ == 0) {
                scroll_down(1);
            } else {
                --row_;
            }
            break;
        case 'c':
            *this = Term(rows_, cols_);
            break;
        default:
            break;
        }
    }

    void csi_byte(unsigned char c) {
        if (c == 0x1b) {  // a sequence cut short by another
            state_ = State::Esc;
            return;
        }
        if (c < 0x20) {
            control(c);
            return;
        }
        if (c >= 0x30 && c <= 0x3f) {
            if (params_.empty() && (c == '?' || c == '>' || c == '<' || c == '=')) {
                private_ = static_cast<char>(c);
            } else {
                params_ += static_cast<char>(c);
            }
            return;
        }
        if (c >= 0x20 && c <= 0x2f) {
            intermediate_ = true;
            return;
        }
        state_ = State::Ground;
        if (c >= 0x40 && c <= 0x7e && !intermediate_) {
            csi(static_cast<char>(c));
        }
    }

    // The numbers in the parameters; ':' counts as ';'.
    std::vector<int> numbers() const {
        std::vector<int> out;
        int n = -1;
        for (char c : params_) {
            if (c >= '0' && c <= '9') {
                n = (n < 0 ? 0 : n) * 10 + (c - '0');
                n = std::min(n, 100000);
            } else if (c == ';' || c == ':') {
                out.push_back(n);
                n = -1;
            }
        }
        out.push_back(n);
        return out;
    }

    // Parameter i, or def when it's missing or 0 where 0 means def.
    int arg(std::size_t i, int def) const {
        std::vector<int> ns = numbers();
        if (i >= ns.size() || ns[i] <= 0) {
            return def;
        }
        return ns[i];
    }

    void move_to(int row, int col) {
        row_ = std::clamp(row, 0, rows_ - 1);
        col_ = std::clamp(col, 0, cols_ - 1);
        wrap_ = false;
    }

    void csi(char final) {
        if (private_ == '?') {
            if (final == 'h' || final == 'l') {
                for (int mode : numbers()) {
                    set_private_mode(mode, final == 'h');
                }
            }
            return;
        }
        if (private_ != 0) {
            return;
        }
        switch (final) {
        case 'A':
            move_to(row_ - arg(0, 1), col_);
            break;
        case 'B':
        case 'e':
            move_to(row_ + arg(0, 1), col_);
            break;
        case 'C':
        case 'a':
            move_to(row_, col_ + arg(0, 1));
            break;
        case 'D':
            move_to(row_, col_ - arg(0, 1));
            break;
        case 'E':
            move_to(row_ + arg(0, 1), 0);
            break;
        case 'F':
            move_to(row_ - arg(0, 1), 0);
            break;
        case 'G':
        case '`':
            move_to(row_, arg(0, 1) - 1);
            break;
        case 'd':
            move_to(arg(0, 1) - 1, col_);
            break;
        case 'H':
        case 'f':
            move_to(arg(0, 1) - 1, arg(1, 1) - 1);
            break;
        case 'J':
            erase_display(std::max(0, numbers()[0]));
            break;
        case 'K':
            erase_line(std::max(0, numbers()[0]));
            break;
        case 'X':
            fill(row_, col_, std::min(cols_, col_ + arg(0, 1)));
            wrap_ = false;
            break;
        case 'P':
            delete_chars(arg(0, 1));
            break;
        case '@':
            insert_chars(arg(0, 1));
            break;
        case 'L':
            insert_lines(arg(0, 1));
            break;
        case 'M':
            delete_lines(arg(0, 1));
            break;
        case 'S':
            scroll_up(std::min(arg(0, 1), rows_));
            break;
        case 'T':
            scroll_down(std::min(arg(0, 1), rows_));
            break;
        case 'm':
            sgr();
            break;
        case 's':
            save_cursor();
            break;
        case 'u':
            restore_cursor();
            break;
        case 'n':
            if (numbers()[0] == 6) {
                replies_ += "\x1b[" + std::to_string(row_ + 1) + ";" + std::to_string(col_ + 1) + "R";
            } else if (numbers()[0] == 5) {
                replies_ += "\x1b[0n";
            }
            break;
        case 'c':
            replies_ += "\x1b[?1;2c";
            break;
        default:
            break;
        }
    }

    void set_private_mode(int mode, bool on) {
        switch (mode) {
        case 25:
            cursor_visible_ = on;
            break;
        case 47:
        case 1047:
        case 1049:
            if (on == on_alt_) {
                break;
            }
            if (on) {
                if (mode == 1049) {
                    save_cursor();
                }
                on_alt_ = true;
                alt_ = blank_grid();
            } else {
                on_alt_ = false;
                if (mode == 1049) {
                    restore_cursor();
                }
            }
            break;
        default:
            break;  // wrapping, mouse, bracketed paste, synchronised output...
        }
    }

    // Cells [from, to) of row r.
    void fill(int r, int from, int to) {
        for (int c = from; c < to; ++c) {
            cell(r, c) = erased();
        }
    }

    // Where an erase to the right starts: past a character just written
    // in the last column, which it leaves alone.
    int erase_from() const { return wrap_ ? cols_ : col_; }

    void erase_line(int mode) {
        if (mode == 0) {
            fill(row_, erase_from(), cols_);
        } else if (mode == 1) {
            fill(row_, 0, col_ + 1);
        } else {
            fill(row_, 0, cols_);
        }
        if (mode != 0) {
            wrap_ = false;
        }
    }

    void erase_display(int mode) {
        if (mode == 0) {
            fill(row_, erase_from(), cols_);
            for (int r = row_ + 1; r < rows_; ++r) {
                fill(r, 0, cols_);
            }
        } else if (mode == 1) {
            for (int r = 0; r < row_; ++r) {
                fill(r, 0, cols_);
            }
            fill(row_, 0, col_ + 1);
            wrap_ = false;
        } else {
            for (int r = 0; r < rows_; ++r) {
                fill(r, 0, cols_);
            }
            wrap_ = false;
        }
    }

    void delete_chars(int n) {
        n = std::min(n, cols_ - col_);
        for (int c = col_; c < cols_; ++c) {
            cell(row_, c) = c + n < cols_ ? cell(row_, c + n) : erased();
        }
        wrap_ = false;
    }

    void insert_chars(int n) {
        n = std::min(n, cols_ - col_);
        for (int c = cols_ - 1; c >= col_; --c) {
            cell(row_, c) = c - n >= col_ ? cell(row_, c - n) : erased();
        }
        wrap_ = false;
    }

    void insert_lines(int n) {
        auto& g = grid();
        n = std::min(n, rows_ - row_);
        for (int i = 0; i < n; ++i) {
            g.erase(g.end() - cols_, g.end());
            g.insert(g.begin() + row_ * cols_, static_cast<std::size_t>(cols_), erased());
        }
        col_ = 0;
        wrap_ = false;
    }

    void delete_lines(int n) {
        auto& g = grid();
        n = std::min(n, rows_ - row_);
        for (int i = 0; i < n; ++i) {
            g.erase(g.begin() + row_ * cols_, g.begin() + (row_ + 1) * cols_);
            g.insert(g.end(), static_cast<std::size_t>(cols_), erased());
        }
        col_ = 0;
        wrap_ = false;
    }

    // 38;5;n and 38;2;r;g;b, from ns[i] on. Moves i past them.
    static int extended_colour(const std::vector<int>& ns, std::size_t& i) {
        if (i + 1 >= ns.size()) {
            return -1;
        }
        if (ns[i + 1] == 5 && i + 2 < ns.size()) {
            int n = std::clamp(ns[i + 2], 0, 255);
            i += 2;
            return n;
        }
        if (ns[i + 1] == 2 && i + 4 < ns.size()) {
            int rgb = (std::clamp(ns[i + 2], 0, 255) << 16) | (std::clamp(ns[i + 3], 0, 255) << 8) |
                      std::clamp(ns[i + 4], 0, 255);
            i += 4;
            return 0x1000000 | rgb;
        }
        ++i;
        return -1;
    }

    void sgr() {
        std::vector<int> ns = numbers();
        for (std::size_t i = 0; i < ns.size(); ++i) {
            int n = std::max(ns[i], 0);  // a missing parameter is 0
            if (n == 0) {
                pen_ = Cell{};
            } else if (n == 1) {
                pen_.bold = true;
            } else if (n == 22) {
                pen_.bold = false;
            } else if (n == 7) {
                pen_.inverse = true;
            } else if (n == 27) {
                pen_.inverse = false;
            } else if (n >= 30 && n <= 37) {
                pen_.fg = n - 30;
            } else if (n == 38) {
                pen_.fg = extended_colour(ns, i);
            } else if (n == 39) {
                pen_.fg = -1;
            } else if (n >= 40 && n <= 47) {
                pen_.bg = n - 40;
            } else if (n == 48) {
                pen_.bg = extended_colour(ns, i);
            } else if (n == 49) {
                pen_.bg = -1;
            } else if (n >= 90 && n <= 97) {
                pen_.fg = n - 90 + 8;
            } else if (n >= 100 && n <= 107) {
                pen_.bg = n - 100 + 8;
            }
            // Italic, underline, blink, strike-through and the rest change
            // nothing the tests look at.
        }
    }

    void save_cursor() {
        saved_row_ = row_;
        saved_col_ = col_;
        saved_pen_ = pen_;
    }

    void restore_cursor() {
        move_to(saved_row_, saved_col_);
        pen_ = saved_pen_;
    }

    int rows_, cols_;
    std::vector<Cell> main_, alt_;
    bool on_alt_ = false;
    int row_ = 0, col_ = 0;
    bool wrap_ = false;  // the last character went in the last column
    bool cursor_visible_ = true;
    Cell pen_;  // the attributes new characters get
    int saved_row_ = 0, saved_col_ = 0;
    Cell saved_pen_;
    State state_ = State::Ground;
    std::string params_;
    char private_ = 0;
    bool intermediate_ = false;
    std::string replies_;
};

}  // namespace ktest
