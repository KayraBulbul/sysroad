#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

struct HexOptions {
    std::uint64_t skip = 0;    // -s: bytes to skip first
    bool has_length = false;   // -n given?
    std::uint64_t length = 0;  // -n: bytes to dump at most
    bool no_squeeze = false;   // -v: print repeated lines too
    std::string error;         // what was wrong with the arguments, or ""
    std::vector<std::string_view> files;
};

// "123" (decimal) or "0x1f" (hex). False if s isn't a whole number.
bool parse_number(std::string_view s, std::uint64_t& out);

HexOptions parse_hex_options(const std::vector<std::string_view>& args);

// One hexdump -C line for up to 16 bytes at `offset`, ending in '\n'.
std::string hex_line(std::uint64_t offset, std::string_view bytes);

// Turns a stream of bytes, fed in pieces of any size, into hexdump -C output.
class Dumper {
  public:
    Dumper(std::ostream& out, const HexOptions& o);

    // More input. Prints every full 16-byte line it completes.
    void feed(std::string_view bytes);

    // The input is over: prints the last partial line and the final offset.
    void finish();

  private:
    void line(std::string_view bytes);  // prints (or squeezes) one line

    std::ostream& out_;
    HexOptions o_;
    std::uint64_t offset_ = 0;  // where the next line starts
    std::uint64_t to_skip_;     // -s bytes still to skip
    std::string pending_;       // bytes waiting for a full line
    std::string prev_;          // the last full line, for squeezing
    bool squeezing_ = false;    // printed "*" for the current run
    bool consumed_ = false;     // read (or skipped) at least one byte
};

// The whole program. Returns the exit status.
int run_hexdump(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
                std::ostream& err);
