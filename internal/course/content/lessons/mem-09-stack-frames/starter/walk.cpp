#include "walk.hpp"

#include <algorithm>
#include <utility>
#include <vector>

// These work, until the input is deep. Rewrite each one with a loop.

long sum_list(const Node* head) {
    if (head == nullptr) {
        return 0;
    }
    return head->value + sum_list(head->next);
}

int tree_depth(const Tree* root) {
    if (root == nullptr) {
        return 0;
    }
    return 1 + std::max(tree_depth(root->left), tree_depth(root->right));
}

long count_nodes(const Tree* root) {
    if (root == nullptr) {
        return 0;
    }
    return 1 + count_nodes(root->left) + count_nodes(root->right);
}
