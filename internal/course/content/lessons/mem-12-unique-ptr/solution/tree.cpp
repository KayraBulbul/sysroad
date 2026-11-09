#include "tree.hpp"

#include <algorithm>

static auto find_slot(Entry& parent, std::string_view name) {
    return std::lower_bound(
        parent.children.begin(), parent.children.end(), name,
        [](const std::unique_ptr<Entry>& e, std::string_view n) { return e->name < n; });
}

Entry* child(Entry& parent, std::string_view name) {
    auto it = find_slot(parent, name);
    if (it != parent.children.end() && (*it)->name == name) {
        return it->get();
    }
    auto e = std::make_unique<Entry>();
    e->name = name;
    return parent.children.insert(it, std::move(e))->get();
}

void add_path(Entry& root, std::string_view path) {
    Entry* at = &root;
    while (!path.empty()) {
        std::size_t slash = path.find('/');
        std::string_view part = path.substr(0, slash);
        path = slash == std::string_view::npos ? "" : path.substr(slash + 1);
        if (!part.empty() && part != ".") {
            at = child(*at, part);
        }
    }
}

std::unique_ptr<Entry> detach(Entry& parent, std::string_view name) {
    auto it = find_slot(parent, name);
    if (it == parent.children.end() || (*it)->name != name) {
        return nullptr;
    }
    std::unique_ptr<Entry> out = std::move(*it);
    parent.children.erase(it);
    return out;
}

static void print_children(const Entry& e, const std::string& prefix, std::ostream& out, int& dirs,
                           int& files) {
    for (std::size_t i = 0; i < e.children.size(); ++i) {
        const Entry& c = *e.children[i];
        bool last = i + 1 == e.children.size();
        out << prefix << (last ? "└── " : "├── ") << c.name << '\n';
        if (c.children.empty()) {
            ++files;
        } else {
            ++dirs;
            print_children(c, prefix + (last ? "    " : "│   "), out, dirs, files);
        }
    }
}

void print_tree(const Entry& root, std::ostream& out) {
    int dirs = 0;
    int files = 0;
    out << root.name << '\n';
    print_children(root, "", out, dirs, files);
    out << '\n'
        << dirs << (dirs == 1 ? " directory, " : " directories, ") << files
        << (files == 1 ? " file" : " files") << '\n';
}
