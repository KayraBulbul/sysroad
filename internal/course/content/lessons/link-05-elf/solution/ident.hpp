#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstdint>
#include <string>
#include <string_view>

// Little-endian numbers at offset `at` in b. Throw std::out_of_range if
// they'd run past the end.
std::uint16_t read_u16(std::string_view b, std::size_t at);
std::uint32_t read_u32(std::string_view b, std::size_t at);
std::uint64_t read_u64(std::string_view b, std::size_t at);

// What kind of file b is, from its first bytes, like file(1):
//
//   "ELF 64-bit little-endian"     also 32-bit, and big-endian
//   "Mach-O 64-bit little-endian"  also 32-bit, and big-endian
//   "Mach-O universal"             a fat binary with code for several CPUs
//   "Java class"                   the same magic number, see the lesson
//   "ar archive"                   "!<arch>\n"
//   "script"                       starts with "#!"
//   "DOS/Windows executable"       starts with "MZ"
//   "unknown"                      anything else, including an ELF header
//                                  whose class or byte order isn't valid
std::string identify(std::string_view b);
