#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

// One hexdump -C line for up to 16 bytes at `offset`, ending in '\n'.
std::string hex_line(std::uint64_t offset, std::string_view bytes);

// Turns a stream of bytes, fed in pieces of any size, into hexdump -C output.
class Dumper {
  public:
    explicit Dumper(std::ostream& out) : out_(out) {}

    // More input. Prints every full 16-byte line it completes.
    void feed(std::string_view bytes);

    // The input is over: prints the last partial line and the final offset.
    void finish();

  private:
    std::ostream& out_;
    std::uint64_t offset_ = 0;  // where the next line starts
    std::string pending_;       // bytes waiting for a full line
};

// The whole program. Returns the exit status.
int run_hexdump(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
                std::ostream& err);
