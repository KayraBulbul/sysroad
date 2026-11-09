// kite [FILE]: edit and save a file. You won't need to change this file.
#include <exception>
#include <iostream>
#include <unistd.h>

#include "editor.hpp"
#include "keys.hpp"
#include "terminal.hpp"

int main(int argc, char** argv) {
    Editor e;
    if (argc > 1) {
        open_file(e, argv[1]);
    }
    set_status(e, "Ctrl-S save · Ctrl-Q quit");
    try {
        RawMode raw(STDIN_FILENO);
        AltScreen alt;
        KeyReader keys(STDIN_FILENO);
        while (true) {
            WindowSize ws = window_size(STDOUT_FILENO);
            e.screen_rows = ws.rows - 2;  // the status and message bars
            e.screen_cols = ws.cols;
            scroll(e);
            write_all(draw_screen(e));
            if (!process_key(e, keys, keys.next())) {
                break;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "kite: " << ex.what() << '\n';
        return 1;
    }
}
