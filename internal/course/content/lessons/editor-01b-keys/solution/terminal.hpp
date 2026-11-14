#pragma once
// The terminal: raw mode, and reading keys.

#include <string>
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
