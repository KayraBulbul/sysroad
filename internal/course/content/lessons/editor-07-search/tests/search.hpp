#pragma once
// Searching through the keys: Ctrl-F, the query, then Enter, Escape or
// the arrows.
#include <string>

#include "session.hpp"

namespace ktest {

// The cats: three lines with "cat" on each, in a 10-row window.
inline Kite cats() {
    Kite k(10, 60);
    k.open("cats.txt", "the cat\na cat sat\ncat\n");
    return k;
}

// Whether the message bar asks the search question, with this query.
inline bool asking(const Kite& k, const std::string& query) {
    std::string bar = k.line(k.term.rows() - 1);
    return bar.rfind("Search", 0) == 0 && bar.size() >= query.size() &&
           bar.compare(bar.size() - query.size(), query.size(), query) == 0;
}

}  // namespace ktest
