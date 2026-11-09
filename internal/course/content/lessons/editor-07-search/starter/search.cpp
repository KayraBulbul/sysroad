#include "editor.hpp"

std::optional<Pos> find(const std::vector<Row>& rows, std::string_view query, Pos from, int dir) {
    // TODO: forwards or backwards from `from`, then wrap around.
    (void)rows;
    (void)query;
    (void)from;
    (void)dir;
    return std::nullopt;
}
