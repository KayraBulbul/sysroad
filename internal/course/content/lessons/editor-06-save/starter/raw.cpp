// The course's version of what you wrote last lesson. You won't need to
// change this file.
#include "terminal.hpp"

termios raw_attrs(termios t) {
    // Input: no Ctrl-S/Ctrl-Q flow control (IXON), no turning Enter's '\r'
    // into '\n' (ICRNL), and the old serial-line flags nobody needs.
    t.c_iflag &= ~static_cast<tcflag_t>(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // Output: no turning "\n" into "\r\n". We write "\r\n" ourselves.
    t.c_oflag &= ~static_cast<tcflag_t>(OPOST);
    t.c_cflag |= CS8;
    // No echo, no line buffering (ICANON), no Ctrl-C/Ctrl-Z signals (ISIG),
    // no Ctrl-V literal-next (IEXTEN).
    t.c_lflag &= ~static_cast<tcflag_t>(ECHO | ICANON | IEXTEN | ISIG);
    // read returns as soon as there's a byte, or after 1/10 s with none.
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 1;
    return t;
}
