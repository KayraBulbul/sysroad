#pragma once
// Test types that count what happens to them.

#include <string>
#include <utility>

struct Counted {
    static inline int live = 0;  // constructed and not yet destroyed
    static inline int copies = 0;
    static inline int moves = 0;
    static void reset() {
        live = copies = moves = 0;
    }

    std::string value;

    Counted() : value("default") {
        ++live;
    }
    explicit Counted(std::string v) : value(std::move(v)) {
        ++live;
    }
    Counted(const Counted& o) : value(o.value) {
        ++live;
        ++copies;
    }
    Counted(Counted&& o) noexcept : value(std::move(o.value)) {
        ++live;
        ++moves;
    }
    Counted& operator=(const Counted& o) {
        value = o.value;
        ++copies;
        return *this;
    }
    Counted& operator=(Counted&& o) noexcept {
        value = std::move(o.value);
        ++moves;
        return *this;
    }
    ~Counted() {
        --live;
    }
};

// No default constructor: a vector must never make one up.
struct NoDefault {
    int n;
    explicit NoDefault(int x) : n(x) {}
};
