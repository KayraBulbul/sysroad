#include "inventory.hpp"

void Inventory::add(const std::string& name, int qty) {
    items_[name] += qty;
}

int Inventory::count(const std::string& name) const {
    auto it = items_.find(name);
    return it == items_.end() ? 0 : it->second;
}

std::size_t Inventory::size() const {
    return items_.size();
}

std::vector<std::string> Inventory::names() const {
    std::vector<std::string> out;
    for (const auto& [name, qty] : items_) {
        out.push_back(name);
    }
    return out;
}

int total(const Inventory& inv) {
    int sum = 0;
    for (const auto& name : inv.names()) {
        sum += inv.count(name);
    }
    return sum;
}
