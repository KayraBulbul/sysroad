#include "terminal.hpp"

termios raw_attrs(termios t) {
    // TODO: switch off echo, line buffering, signals, Ctrl-S/Ctrl-Q flow
    // control, CR-to-NL translation and output processing. Then make read
    // return after a tenth of a second, even with nothing to read.
    return t;
}
