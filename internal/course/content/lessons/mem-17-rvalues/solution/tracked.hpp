#pragma once
// A string that counts how often it's copied and moved. You won't need to change this file.

#include <string>
#include <utility>

struct Tracked {
    static inline int copies = 0;
    static inline int moves = 0;
    static void reset() {
        copies = 0;
        moves = 0;
    }

    std::string value;

    Tracked() = default;
    explicit Tracked(std::string v) : value(std::move(v)) {}
    Tracked(const Tracked& other) : value(other.value) {
        ++copies;
    }
    Tracked(Tracked&& other) noexcept : value(std::move(other.value)) {
        ++moves;
    }
    Tracked& operator=(const Tracked& other) {
        value = other.value;
        ++copies;
        return *this;
    }
    Tracked& operator=(Tracked&& other) noexcept {
        value = std::move(other.value);
        ++moves;
        return *this;
    }
};
