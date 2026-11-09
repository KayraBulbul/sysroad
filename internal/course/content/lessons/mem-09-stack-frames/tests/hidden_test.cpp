#include "doctest.h"

#include <vector>

#include "walk.hpp"

TEST_CASE("empty trees") {
    CHECK(tree_depth(nullptr) == 0);
    CHECK(count_nodes(nullptr) == 0);
    Tree one{nullptr, nullptr};
    CHECK(tree_depth(&one) == 1);
    CHECK(count_nodes(&one) == 1);
}

TEST_CASE("the deepest path can be on either side") {
    // root -> right -> right -> left, plus a short left branch
    Tree l1{nullptr, nullptr};
    Tree deep{nullptr, nullptr};
    Tree r2{&deep, nullptr};
    Tree r1{nullptr, &r2};
    Tree root{&l1, &r1};
    CHECK(tree_depth(&root) == 4);
    CHECK(count_nodes(&root) == 5);
}

TEST_CASE("a full binary tree") {
    // 2^16 - 1 nodes, depth 16, stored heap-style: children of i are 2i+1 and 2i+2.
    const int n = (1 << 16) - 1;
    std::vector<Tree> t(n);
    for (int i = 0; i < n; ++i) {
        t[i] = {2 * i + 1 < n ? &t[2 * i + 1] : nullptr, 2 * i + 2 < n ? &t[2 * i + 2] : nullptr};
    }
    CHECK(tree_depth(t.data()) == 16);
    CHECK(count_nodes(t.data()) == n);
}

TEST_CASE("a deep zigzag") {
    const long n = 500'000;
    std::vector<Tree> chain(n);
    for (long i = 0; i < n; ++i) {
        Tree* next = i + 1 < n ? &chain[i + 1] : nullptr;
        chain[i] = i % 2 == 0 ? Tree{next, nullptr} : Tree{nullptr, next};
    }
    CHECK(tree_depth(chain.data()) == n);
    CHECK(count_nodes(chain.data()) == n);
}

TEST_CASE("negative values") {
    Node b{-5, nullptr}, a{2, &b};
    CHECK(sum_list(&a) == -3);
}
