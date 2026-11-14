#pragma once
// The terminal: raw mode.

#include <termios.h>

// Puts the terminal on fd in raw mode for as long as it lives, then puts
// the old settings back. Throws std::runtime_error if fd isn't a terminal.
class RawMode {
  public:
    explicit RawMode(int fd);
    ~RawMode();
    RawMode(const RawMode&) = delete;
    RawMode& operator=(const RawMode&) = delete;

  private:
    int fd_;
    termios saved_;
};
