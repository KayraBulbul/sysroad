// kite [FILE]: shows the file until Ctrl-Q.
#include <exception>
#include <iostream>
#include <unistd.h>

#include "kite.hpp"
#include "terminal.hpp"

int main(int argc, char** argv) {
    Editor e;
    set_status(e, "Ctrl-Q quit");
    if (argc > 1) {
        open_file(e, argv[1]);  // a new file says so instead
    }
    try {
        RawMode raw(STDIN_FILENO);
        AltScreen alt;
        KeyReader keys(STDIN_FILENO);
        while (true) {
            WindowSize ws = window_size(STDOUT_FILENO);
            set_window_size(e, ws.rows, ws.cols);
            write_all(draw_screen(e));
            if (keys.next() == ctrl_key('q')) {
                break;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "kite: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
