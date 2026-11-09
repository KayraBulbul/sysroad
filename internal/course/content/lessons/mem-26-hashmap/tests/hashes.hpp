#pragma once
// Hash functions for testing collisions.

#include <cstddef>

// Every key collides.
struct ZeroHash {
    std::size_t operator()(int) const noexcept {
        return 0;
    }
};

// Keys hash to themselves, minus one, so key 0 lands in the very last slot
// and its neighbours wrap around to slot 0.
struct MinusOneHash {
    std::size_t operator()(int k) const noexcept {
        return static_cast<std::size_t>(k) - 1;
    }
};
