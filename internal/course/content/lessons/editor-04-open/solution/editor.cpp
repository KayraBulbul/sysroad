#include <algorithm>
#include <fstream>

#include "kite.hpp"

void set_window_size(Editor& e, int rows, int cols) {
    e.screen_rows = std::max(rows - 2, 0);  // the status bar and the message bar
    e.screen_cols = cols;
}

void open_file(Editor& e, const std::string& path) {
    e.filename = path;
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        set_status(e, "New file");
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        Row row{line, ""};
        update_render(row);
        e.rows.push_back(std::move(row));
    }
}

void set_status(Editor& e, std::string msg) {
    e.status_msg = std::move(msg);
    e.status_time = std::time(nullptr);
}
