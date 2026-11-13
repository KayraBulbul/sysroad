#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <csignal>
#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

// Writes "<text><n>\n" into buf, as much of it as fits in size bytes,
// using nothing that isn't async-signal-safe. Returns the bytes written.
size_t format_line(char* buf, size_t size, const char* text, long n);

// Turns signals into bytes on a pipe, so a poll loop can wait for them
// along with its other fds. Only one can exist at a time.
class SignalPipe {
  public:
    // Makes the pipe, both ends non-blocking and close-on-exec, and
    // installs a handler with SA_RESTART for each of sigs.
    explicit SignalPipe(std::initializer_list<int> sigs);
    // Puts the old actions back and closes the pipe.
    ~SignalPipe();
    SignalPipe(const SignalPipe&) = delete;
    SignalPipe& operator=(const SignalPipe&) = delete;

    // The read end, to poll.
    int fd() const {
        return read_fd_;
    }
    // The next signal caught, oldest first, or 0 if none are waiting.
    int next();

  private:
    std::vector<std::pair<int, struct sigaction>> old_;
    int read_fd_ = -1;
};
