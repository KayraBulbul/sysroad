#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <istream>
#include <ostream>
#include <string_view>
#include <vector>

struct CatOptions {
    bool number = false;            // -n: number every line
    bool number_nonblank = false;   // -b: number non-empty lines (wins over -n)
    bool squeeze = false;           // -s: squeeze repeated empty lines into one
    bool show_ends = false;         // -E: '$' before each newline
    bool show_tabs = false;         // -T: tabs as ^I
    bool show_nonprinting = false;  // -v: control bytes as ^X, high bytes as M-
    char bad = 0;                   // an unknown flag letter, or 0
    std::vector<std::string_view> files;
};

// What carries over from one file to the next.
struct CatState {
    std::size_t line = 0;       // the last line number printed
    bool at_line_start = true;  // the next byte starts a line
    int blank_run = 0;          // empty lines in a row so far
    bool pending_cr = false;    // -E: a '\r' held back until we see what follows
};

CatOptions parse_cat_options(const std::vector<std::string_view>& args);

// True when no flag changes the output, so a plain copy will do.
bool plain(const CatOptions& o);

// Copies every byte of `in` to `out`. Returns how many it copied.
std::size_t copy_stream(std::istream& in, std::ostream& out);

// Copies `in` to `out`, applying the flags. `st` carries over between files.
void cat_stream(std::istream& in, std::ostream& out, const CatOptions& o, CatState& st);

// The whole program. Returns the exit status.
int run_cat(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
            std::ostream& err);
