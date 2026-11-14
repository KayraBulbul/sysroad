#include <algorithm>
#include <memory>

#include "kite.hpp"

namespace {

// The first match in row r at or after column col, or npos.
std::size_t first_at_or_after(const Row& r, std::string_view query, int col) {
    if (static_cast<std::size_t>(col) > r.chars.size()) {
        return std::string::npos;
    }
    return r.chars.find(query, static_cast<std::size_t>(col));
}

// The last match in row r starting at or before column col, or npos.
std::size_t last_at_or_before(const Row& r, std::string_view query, int col) {
    if (col < 0) {
        return std::string::npos;
    }
    return r.chars.rfind(query, static_cast<std::size_t>(col));
}

}  // namespace

std::optional<Pos> find(const std::vector<Row>& rows, std::string_view query, Pos from, int dir) {
    if (query.empty() || rows.empty()) {
        return std::nullopt;
    }
    int n = static_cast<int>(rows.size());
    if (dir >= 0) {
        for (int r = std::max(from.row, 0); r < n; ++r) {
            std::size_t c = first_at_or_after(rows[r], query, r == from.row ? std::max(from.col, 0) : 0);
            if (c != std::string::npos) {
                return Pos{r, static_cast<int>(c)};
            }
        }
        for (int r = 0; r < n; ++r) {  // wrap around to the top
            std::size_t c = rows[r].chars.find(query);
            if (c != std::string::npos) {
                return Pos{r, static_cast<int>(c)};
            }
        }
        return std::nullopt;
    }
    for (int r = std::min(from.row, n - 1); r >= 0; --r) {
        std::size_t c = r == from.row ? last_at_or_before(rows[r], query, from.col) : rows[r].chars.rfind(query);
        if (c != std::string::npos) {
            return Pos{r, static_cast<int>(c)};
        }
    }
    for (int r = n - 1; r >= 0; --r) {  // wrap around to the bottom
        std::size_t c = rows[r].chars.rfind(query);
        if (c != std::string::npos) {
            return Pos{r, static_cast<int>(c)};
        }
    }
    return std::nullopt;
}

namespace {

// What a search remembers between keys.
struct Search {
    int cx, cy, rowoff, coloff;  // where it started, to go back to
    std::optional<Pos> match;

    void go_back(Editor& e) const {
        e.cx = cx;
        e.cy = cy;
        e.rowoff = rowoff;
        e.coloff = coloff;
    }
};

}  // namespace

void start_search(Editor& e) {
    // Both callbacks share the search's state, which outlives this call.
    auto s = std::make_shared<Search>(Search{e.cx, e.cy, e.rowoff, e.coloff, std::nullopt});
    Prompt p;
    p.label = "Search (arrows: next/previous, Esc: cancel): ";
    p.on_key = [s](Editor& e, const std::string& query, int key) {
        Pos from{s->cy, s->cx};
        int dir = 1;
        if (s->match && (key == kArrowRight || key == kArrowDown)) {
            from = {s->match->row, s->match->col + 1};
        } else if (s->match && (key == kArrowLeft || key == kArrowUp)) {
            from = {s->match->row, s->match->col - 1};
            dir = -1;
        }
        s->match = find(e.rows, query, from, dir);
        if (s->match) {
            e.cy = s->match->row;
            e.cx = s->match->col;
        } else {
            s->go_back(e);
        }
    };
    p.done = [s](Editor& e, std::optional<std::string> answer) {
        if (!answer) {
            s->go_back(e);
        } else if (!s->match) {
            set_status(e, "Not found");
        }
    };
    e.prompt = std::move(p);
}
