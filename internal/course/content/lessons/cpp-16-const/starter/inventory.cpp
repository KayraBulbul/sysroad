#include "inventory.hpp"

void Inventory::add(const std::string& name, int qty) {
    items_[name] += qty;
}

int Inventory::count(const std::string& name) {
    return items_[name];
}

std::size_t Inventory::size() {
    return items_.size();
}

std::vector<std::string> Inventory::names() {
    std::vector<std::string> out;
    for (auto& [name, qty] : items_) {
        out.push_back(name);
    }
    return out;
}

int total(Inventory inv) {
    int sum = 0;
    for (auto& name : inv.names()) {
        sum += inv.count(name);
    }
    return sum;
}
