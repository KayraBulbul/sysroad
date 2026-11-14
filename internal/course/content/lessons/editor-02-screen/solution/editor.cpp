#include <fstream>

#include "kite.hpp"

void set_window_size(Editor& e, int rows, int cols) {
    e.screen_rows = rows;
    e.screen_cols = cols;
}

void open_file(Editor& e, const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::string line;
    while (std::getline(in, line)) {
        e.rows.push_back(Row{line});
    }
}
