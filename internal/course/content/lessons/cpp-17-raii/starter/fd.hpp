#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <string>

// Owns one POSIX file descriptor and closes it when destroyed.
class Fd {
  public:
    Fd() = default;
    explicit Fd(int fd) : fd_(fd) {}
    ~Fd();

    Fd(const Fd&) = delete;  // two owners would close it twice
    Fd& operator=(const Fd&) = delete;

    int get() const {
        return fd_;
    }
    bool valid() const {
        return fd_ >= 0;
    }

    // Stops owning the fd and returns it; the caller must close it.
    int release();

    // Closes the current fd (if any) and owns `fd` instead.
    void reset(int fd = -1);

  private:
    int fd_ = -1;
};

// Opens path read-only. Not valid() on failure, with errno set.
Fd open_read(const char* path);

// Everything from the fd's current position to the end of the file.
std::string read_all(const Fd& fd);
