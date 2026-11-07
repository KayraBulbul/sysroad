#pragma once
// This lesson's one exception: you'll need to change this file.

#include <cstddef>
#include <map>
#include <string>
#include <vector>

class Inventory {
  public:
    // Adds qty of an item (added to any it already has).
    void add(const std::string& name, int qty);

    // How many of an item there are; 0 if it was never added.
    int count(const std::string& name);

    // How many different items there are.
    std::size_t size();

    // Every item's name, in alphabetical order.
    std::vector<std::string> names();

  private:
    std::map<std::string, int> items_;
};

// Every item's count, added up.
int total(Inventory inv);

// n kibibytes in bytes.
inline std::size_t kib(std::size_t n) {
    return n * 1024;
}
