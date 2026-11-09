// kite [FILE]: shows the file (or the welcome screen), and the arrow keys
// move around it until Ctrl-Q. You won't need to change this file.
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
        std::ifstream in(argv[1]);
        std::string line;
        while (std::getline(in, line)) {
            e.rows.push_back(Row{line, line});
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
            write_all(draw_screen(e));
            int key = keys.next();
            if (key == ctrl_key('q')) {
                break;
            }
            move_cursor(e, key);
        }
    } catch (const std::exception& ex) {
        std::cerr << "kite: " << ex.what() << '\n';
        return 1;
    }
}
