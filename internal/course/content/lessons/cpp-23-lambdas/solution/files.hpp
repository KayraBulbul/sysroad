#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct Entry {
    std::string name;
    std::uint64_t size = 0;
};

// Biggest first; equal sizes in name order.
void sort_by_size(std::vector<Entry>& v);

std::uint64_t total_size(const std::vector<Entry>& v);

// How many entries are at least `min` bytes.
std::size_t count_at_least(const std::vector<Entry>& v, std::uint64_t min);

// The biggest entry (the first one on a tie), or nullopt when v is empty.
std::optional<Entry> largest(const std::vector<Entry>& v);

// Removes every entry of size 0.
void remove_empty(std::vector<Entry>& v);

// The names ending in ext, e.g. ".txt", in their original order.
std::vector<std::string> names_with_ext(const std::vector<Entry>& v, std::string_view ext);
