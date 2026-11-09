#pragma once
// Shared by your code and the tests. You won't need to change this file.

struct Node {
    long value;
    Node* next;
};

struct Tree {
    Tree* left;
    Tree* right;
};

// The sum of every value in the list.
long sum_list(const Node* head);

// The number of nodes on the longest path from root down to a leaf; 0 for
// an empty tree.
int tree_depth(const Tree* root);

// How many nodes the tree has.
long count_nodes(const Tree* root);
