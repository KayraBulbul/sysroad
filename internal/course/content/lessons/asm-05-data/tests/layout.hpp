#pragma once
// Builds Items from raw bytes, the way they'd arrive from a file written by
// the code in the listing.

#include <cstring>
#include <vector>

#include "mystery.hpp"

struct Raw {
    int id;
    short shelf;
    char tag;
    long price;
    int count;
};

inline std::vector<Item> items_from(const std::vector<Raw>& raws) {
    std::vector<Item> out(raws.size());
    if (sizeof(Item) != 24) {
        return out;  // the test reports the size
    }
    for (std::size_t i = 0; i < raws.size(); ++i) {
        unsigned char bytes[24] = {};
        std::memcpy(bytes + 0, &raws[i].id, 4);
        std::memcpy(bytes + 4, &raws[i].shelf, 2);
        std::memcpy(bytes + 6, &raws[i].tag, 1);
        std::memcpy(bytes + 8, &raws[i].price, 8);
        std::memcpy(bytes + 16, &raws[i].count, 4);
        std::memcpy(&out[i], bytes, 24);
    }
    return out;
}
