#pragma once
// The terminal: raw mode, reading keys, drawing, its size.

#include <string>
#include <string_view>
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

// Reads keys from a terminal in raw mode.
class KeyReader {
  public:
    explicit KeyReader(int fd) : fd_(fd) {}
    int next();  // waits for the next key

  private:
    bool fill(bool wait);
    int fd_;
    std::string pending_;  // bytes read but not decoded yet
};

// Switches to the terminal's alternate screen for as long as it lives, the
// way vim and less do: when it ends, the shell's screen comes back.
class AltScreen {
  public:
    AltScreen();
    ~AltScreen();
    AltScreen(const AltScreen&) = delete;
    AltScreen& operator=(const AltScreen&) = delete;
};

struct WindowSize {
    int rows = 24;
    int cols = 80;
};

// The size of the terminal on fd, or 24 x 80 if it won't say.
WindowSize window_size(int fd);

// Writes all of s to standard output.
void write_all(std::string_view s);
