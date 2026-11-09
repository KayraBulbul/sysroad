#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <string>
#include <vector>

// Owns one POSIX file descriptor and closes it when destroyed.
class Fd {
  public:
    Fd() = default;
    explicit Fd(int fd) : fd_(fd) {}
    ~Fd();

    Fd(const Fd&) = delete;  // two owners would close it twice
    Fd& operator=(const Fd&) = delete;

    Fd(Fd&& other) noexcept;             // takes other's fd; other is left empty
    Fd& operator=(Fd&& other) noexcept;  // closes this fd, then takes other's

    int get() const {
        return fd_;
    }
    bool valid() const {
        return fd_ >= 0;
    }
    int release();
    void reset(int fd = -1);

  private:
    int fd_ = -1;
};

// From lesson 1.3.4: opens path read-only (not valid() on failure), and reads everything.
Fd open_read(const char* path);
std::string read_all(const Fd& fd);

// Opens each path. The ones that open are in the result, in order; the
// others are appended to failed.
std::vector<Fd> open_all(const std::vector<std::string>& paths, std::vector<std::string>& failed);

// The first of paths that opens, or an empty Fd if none do.
Fd open_first(const std::vector<std::string>& paths);
