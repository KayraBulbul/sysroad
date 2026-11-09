#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <string>
#include <vector>

// A field as the compiler sees it.
struct Field {
    std::string name;
    std::size_t size;
    std::size_t align;  // a power of two
};

// A field where lay_out put it.
struct Placed {
    std::string name;
    std::size_t offset;
    std::size_t size;
    std::size_t padding_before;  // gap between the previous field and this one
};

struct Layout {
    std::vector<Placed> fields;
    std::size_t size = 0;          // sizeof the struct
    std::size_t align = 1;         // alignof the struct
    std::size_t tail_padding = 0;  // gap after the last field
};

// n rounded up to a multiple of align (a power of two).
std::size_t align_up(std::size_t n, std::size_t align);

// Places the fields in order, the way a C++ compiler does. At least one field.
Layout lay_out(const std::vector<Field>& fields);
