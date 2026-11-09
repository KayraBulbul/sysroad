#include <stdexcept>

#include "editor.hpp"

std::string rows_to_string(const std::vector<Row>& rows) {
    // TODO
    (void)rows;
    throw std::logic_error("rows_to_string isn't written yet");
}

bool save_file(Editor& e) {
    // TODO: write to a temporary file next to it, then rename it over.
    set_status(e, "Can't save! save_file isn't written yet");
    return false;
}

std::string status_bar(const Editor& e) {
    // TODO
    return std::string(static_cast<std::size_t>(e.screen_cols), ' ');
}
