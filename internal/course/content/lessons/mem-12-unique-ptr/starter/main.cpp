// tree [PATH]: draw the paths read from standard input as a tree, or just
// the part under PATH. You won't need to change this file.
#include <iostream>
#include <string>

#include "tree.hpp"

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "usage: tree [PATH] < paths\n";
        return 2;
    }
    auto root = std::make_unique<Entry>();
    root->name = ".";
    std::string line;
    while (std::getline(std::cin, line)) {
        add_path(*root, line);
    }
    if (argc == 2) {
        // Walk down to PATH's parent, then take PATH out of the tree.
        std::string path = argv[1];
        Entry* parent = root.get();
        std::size_t start = 0;
        while (parent != nullptr) {
            std::size_t slash = path.find('/', start);
            std::string part = path.substr(start, slash - start);
            if (slash == std::string::npos) {
                root = detach(*parent, part);
                break;
            }
            Entry* next = nullptr;
            for (auto& c : parent->children) {
                if (c->name == part) {
                    next = c.get();
                }
            }
            parent = next;
            start = slash + 1;
        }
        if (parent == nullptr || root == nullptr) {
            std::cerr << "tree: " << path << ": not in the list\n";
            return 1;
        }
        root->name = path;
    }
    print_tree(*root, std::cout);
}
