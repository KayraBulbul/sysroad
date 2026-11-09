#pragma once
// Reorder these fields so sizeof(Record) is 24. Keep every field and its type.

#include <cstdint>

struct Record {
    bool active;
    double score;
    char grade;
    std::int64_t id;
    std::int16_t year;
    float weight;
};
