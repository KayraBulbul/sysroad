#pragma once
#include <initializer_list>
#include <optional>
#include <string>
#include <vector>

#include "doctest.h"
#include "editor.hpp"

inline std::vector<Row> lines(std::initializer_list<std::string> ls) {
    std::vector<Row> rows;
    for (const auto& l : ls) {
        rows.push_back(Row{l, l});
    }
    return rows;
}

// "row:col", or "none", so a failure shows both numbers.
inline std::string at(std::optional<Pos> p) {
    return p ? std::to_string(p->row) + ":" + std::to_string(p->col) : "none";
}
