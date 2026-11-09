#include "pack.hpp"

#include <algorithm>

std::vector<Field> packed(std::vector<Field> fields) {
    std::stable_sort(fields.begin(), fields.end(),
                     [](const Field& a, const Field& b) { return a.align > b.align; });
    return fields;
}
