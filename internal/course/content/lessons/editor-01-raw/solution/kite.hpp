#pragma once
// kite, a small text editor. The tests use what this header declares.

#include <termios.h>

// The settings for raw mode, made from the terminal's current ones.
termios raw_attrs(termios t);
