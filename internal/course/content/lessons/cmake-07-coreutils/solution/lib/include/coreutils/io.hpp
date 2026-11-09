#pragma once
// What the tools share. You won't need to change this file.

#include <functional>
#include <istream>
#include <span>
#include <string_view>

namespace coreutils {

// Calls each(in, name) for every file in files, in order, or once for
// standard input when files is empty. "-" also means standard input. A file
// that can't be opened is reported as "TOOL: NAME: can't open" on stderr
// and skipped. Returns the exit status: 0, or 1 if any file failed.
int for_each_input(std::string_view tool, std::span<char* const> files,
                   const std::function<void(std::istream& in, std::string_view name)>& each);

// The arguments after the program's name.
inline std::span<char* const> args(int argc, char** argv) {
    return {argv + 1, static_cast<std::size_t>(argc - 1)};
}

}  // namespace coreutils
