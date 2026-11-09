// The course's version of what you wrote last lesson. You won't need to
// change this file.
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <format>
#include <fstream>

#include "editor.hpp"

std::string rows_to_string(const std::vector<Row>& rows) {
    std::string text;
    for (const Row& row : rows) {
        text += row.chars;
        text += '\n';
    }
    return text;
}

bool save_file(Editor& e) {
    std::string text = rows_to_string(e.rows);
    // Write a new file, then rename it over the old one. A rename is all
    // or nothing, so a crash or a full disk mid-save can't leave half a file.
    std::string tmp = e.filename + ".kite-save";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        out << text;
        out.close();
        if (!out) {
            int err = errno;
            std::remove(tmp.c_str());
            set_status(e, std::format("Can't save! {}", std::strerror(err)));
            return false;
        }
    }
    if (std::rename(tmp.c_str(), e.filename.c_str()) != 0) {
        int err = errno;
        std::remove(tmp.c_str());
        set_status(e, std::format("Can't save! {}", std::strerror(err)));
        return false;
    }
    e.dirty = false;
    set_status(e, std::format("{} bytes written to disk", text.size()));
    return true;
}

std::string status_bar(const Editor& e) {
    auto width = static_cast<std::size_t>(e.screen_cols);
    std::string name = e.filename.empty() ? "[No Name]" : e.filename;
    std::string left =
        std::format("{:.20} - {} lines{}", name, e.rows.size(), e.dirty ? " (modified)" : "");
    std::string right = std::format("{}/{}", e.cy + 1, e.rows.size());
    if (left.size() > width) {
        left.resize(width);
    }
    if (left.size() + right.size() <= width) {
        return left + std::string(width - left.size() - right.size(), ' ') + right;
    }
    return left + std::string(width - left.size(), ' ');
}
