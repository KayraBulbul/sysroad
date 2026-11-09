// From the last lesson. You won't need to change this file.
#include "layout.hpp"

#include <algorithm>

std::size_t align_up(std::size_t n, std::size_t align) {
    return (n + align - 1) & ~(align - 1);
}

Layout lay_out(const std::vector<Field>& fields) {
    Layout out;
    std::size_t offset = 0;
    for (const Field& f : fields) {
        std::size_t at = align_up(offset, f.align);
        out.fields.push_back({f.name, at, f.size, at - offset});
        offset = at + f.size;
        out.align = std::max(out.align, f.align);
    }
    out.size = align_up(offset, out.align);
    out.tail_padding = out.size - offset;
    return out;
}
