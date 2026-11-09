#include "walk.hpp"

#include <algorithm>
#include <utility>
#include <vector>

long sum_list(const Node* head) {
    long total = 0;
    for (const Node* n = head; n != nullptr; n = n->next) {
        total += n->value;
    }
    return total;
}

int tree_depth(const Tree* root) {
    std::vector<std::pair<const Tree*, int>> todo;
    if (root != nullptr) {
        todo.push_back({root, 1});
    }
    int deepest = 0;
    while (!todo.empty()) {
        auto [t, d] = todo.back();
        todo.pop_back();
        deepest = std::max(deepest, d);
        if (t->left != nullptr) {
            todo.push_back({t->left, d + 1});
        }
        if (t->right != nullptr) {
            todo.push_back({t->right, d + 1});
        }
    }
    return deepest;
}

long count_nodes(const Tree* root) {
    std::vector<const Tree*> todo;
    if (root != nullptr) {
        todo.push_back(root);
    }
    long count = 0;
    while (!todo.empty()) {
        const Tree* t = todo.back();
        todo.pop_back();
        ++count;
        if (t->left != nullptr) {
            todo.push_back(t->left);
        }
        if (t->right != nullptr) {
            todo.push_back(t->right);
        }
    }
    return count;
}
