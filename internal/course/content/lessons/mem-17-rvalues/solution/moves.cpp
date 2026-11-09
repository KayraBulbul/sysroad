#include "moves.hpp"

void append(std::vector<Tracked>& out, Tracked item) {
    out.push_back(std::move(item));
}

Tracked take_last(std::vector<Tracked>& v) {
    Tracked t = std::move(v.back());
    v.pop_back();
    return t;
}

std::vector<Tracked> concat(std::vector<Tracked> a, std::vector<Tracked> b) {
    for (Tracked& t : b) {
        a.push_back(std::move(t));
    }
    return a;
}

void rotate3(Tracked& a, Tracked& b, Tracked& c) {
    Tracked tmp = std::move(a);
    a = std::move(b);
    b = std::move(c);
    c = std::move(tmp);
}

const Tracked& longest(const std::vector<Tracked>& v) {
    const Tracked* best = &v[0];
    for (const Tracked& t : v) {
        if (t.value.size() > best->value.size()) {
            best = &t;
        }
    }
    return *best;
}
