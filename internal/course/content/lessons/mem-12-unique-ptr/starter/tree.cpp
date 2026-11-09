#include "tree.hpp"

#include <algorithm>

Entry* child(Entry& parent, std::string_view name) {
    // TODO: find name with std::lower_bound, or insert a new Entry there.
    (void)parent;
    (void)name;
    return nullptr;
}

void add_path(Entry& root, std::string_view path) {
    // TODO
    (void)root;
    (void)path;
}

std::unique_ptr<Entry> detach(Entry& parent, std::string_view name) {
    // TODO
    (void)parent;
    (void)name;
    return nullptr;
}

void print_tree(const Entry& root, std::ostream& out) {
    // TODO
    out << root.name << '\n';
}
