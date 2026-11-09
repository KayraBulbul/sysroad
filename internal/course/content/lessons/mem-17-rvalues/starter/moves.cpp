#include "moves.hpp"

// Each of these works, but copies when it doesn't need to.

void append(std::vector<Tracked>& out, Tracked item) {
    out.push_back(item);
}

Tracked take_last(std::vector<Tracked>& v) {
    Tracked t = v.back();
    v.pop_back();
    return t;
}

std::vector<Tracked> concat(std::vector<Tracked> a, std::vector<Tracked> b) {
    for (const Tracked& t : b) {
        a.push_back(t);
    }
    return a;
}

void rotate3(Tracked& a, Tracked& b, Tracked& c) {
    Tracked tmp = a;
    a = b;
    b = c;
    c = tmp;
}

const Tracked& longest(const std::vector<Tracked>& v) {
    std::size_t best = 0;
    std::size_t i = 0;
    for (Tracked t : v) {
        if (t.value.size() > v[best].value.size()) {
            best = i;
        }
        ++i;
    }
    return v[best];
}
