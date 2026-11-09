#pragma once
// Reorder these fields so sizeof(Record) is 24. Keep every field and its type.

#include <cstdint>

struct Record {
    double score;
    std::int64_t id;
    float weight;
    std::int16_t year;
    bool active;
    char grade;
};
