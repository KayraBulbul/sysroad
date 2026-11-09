#pragma once
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <string>

#include "editor.hpp"

inline Editor with_lines(std::initializer_list<std::string> lines, int cols = 40) {
    Editor e;
    e.screen_cols = cols;
    for (const auto& l : lines) {
        Row r{l, ""};
        update_render(r);
        e.rows.push_back(r);
    }
    return e;
}

inline std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}
