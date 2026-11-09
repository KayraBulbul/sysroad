#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

// One name in the tree. Its children are kept sorted by name.
struct Entry {
    std::string name;
    std::vector<std::unique_ptr<Entry>> children;
};

// parent's child called name, inserted in sorted position if it isn't
// there yet. The pointer stays valid when more children are added.
Entry* child(Entry& parent, std::string_view name);

// Adds every part of a path like "src/util/str.cpp" under root, skipping
// empty parts and ".".
void add_path(Entry& root, std::string_view path);

// Takes the child called name, and everything under it, out of parent and
// hands it over. nullptr if there's no such child.
std::unique_ptr<Entry> detach(Entry& parent, std::string_view name);

// root's name, the tree under it, a blank line and "N directories, M files".
// root itself counts as a directory, as it does in tree.
void print_tree(const Entry& root, std::ostream& out);
