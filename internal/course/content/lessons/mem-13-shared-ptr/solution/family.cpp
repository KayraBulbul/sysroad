#include "family.hpp"

void adopt(const std::shared_ptr<Person>& parent, const std::shared_ptr<Person>& child) {
    parent->children.push_back(child);
    child->parent = parent;
}

std::string lineage(const Person& p) {
    std::string out = p.name;
    for (auto up = p.parent.lock(); up; up = up->parent.lock()) {
        out += " < " + up->name;
    }
    return out;
}
