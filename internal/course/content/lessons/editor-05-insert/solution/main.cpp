// kite [FILE]: edit the file (but not save it, yet) until Ctrl-Q. You won't
// need to change this file.
#include <exception>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "editor.hpp"
#include "keys.hpp"
#include "terminal.hpp"

int main(int argc, char** argv) {
    Editor e;
    if (argc > 1) {
        std::ifstream in(argv[1], std::ios::binary);
        if (in) {
            e.rows = read_rows(in);
        }
    }
    try {
        RawMode raw(STDIN_FILENO);
        AltScreen alt;
        KeyReader keys(STDIN_FILENO);
        while (true) {
            WindowSize ws = window_size(STDOUT_FILENO);
            e.screen_rows = ws.rows;
            e.screen_cols = ws.cols;
            scroll(e);
            write_all(draw_screen(e));
            if (!process_key(e, keys.next())) {
                break;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "kite: " << ex.what() << '\n';
        return 1;
    }
}
