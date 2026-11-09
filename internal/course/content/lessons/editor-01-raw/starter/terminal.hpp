#pragma once
// The terminal: raw mode, reading keys, its size. You won't need to change
// this file.

#include <string>
#include <string_view>
#include <termios.h>

// The settings for raw mode, made from the terminal's current ones.
termios raw_attrs(termios t);

// Puts the terminal in raw mode for as long as it lives, then restores it.
// Throws std::runtime_error if fd isn't a terminal.
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
    const std::string& last_bytes() const {
        return last_;
    }

  private:
    bool fill(bool wait);
    int fd_;
    std::string pending_;
    std::string last_;
};

struct WindowSize {
    int rows = 24;
    int cols = 80;
};
WindowSize window_size(int fd);

// Writes all of s to standard output.
void write_all(std::string_view s);
