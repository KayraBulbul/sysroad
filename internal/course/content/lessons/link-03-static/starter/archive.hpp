#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct Member {
    std::string name;
    std::size_t header_offset = 0;  // where its 60-byte header starts in the file
    std::string_view data;          // its contents, pointing into the archive's bytes
};

struct Archive {
    // The files in the archive, in order. The symbol table and GNU's
    // long-name table aren't members.
    std::vector<Member> members;
    // The symbol index: each symbol, and the name of the member defining
    // it, in the order the table lists them. Empty if there's no table.
    std::vector<std::pair<std::string, std::string>> symbols;
};

// Reads a GNU or BSD archive. Throws std::runtime_error if it isn't one,
// or it's cut short.
Archive parse_archive(std::string_view bytes);
