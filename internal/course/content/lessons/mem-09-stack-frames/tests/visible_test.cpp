#include "doctest.h"

#include <vector>

#include "walk.hpp"

TEST_CASE("small inputs") {
    Node c{3, nullptr}, b{2, &c}, a{1, &b};
    CHECK(sum_list(&a) == 6);
    CHECK(sum_list(nullptr) == 0);

    Tree leaf1{nullptr, nullptr}, leaf2{nullptr, nullptr};
    Tree mid{&leaf1, nullptr};
    Tree root{&mid, &leaf2};
    CHECK(tree_depth(&root) == 3);
    CHECK(count_nodes(&root) == 4);
}

TEST_CASE("a million-node list") {
    const long n = 1'000'000;
    std::vector<Node> list(n);
    for (long i = 0; i < n; ++i) {
        list[i] = {1, i + 1 < n ? &list[i + 1] : nullptr};
    }
    CHECK(sum_list(list.data()) == n);
}

TEST_CASE("a million-deep tree") {
    const long n = 1'000'000;
    std::vector<Tree> chain(n);
    for (long i = 0; i < n; ++i) {
        chain[i] = {i + 1 < n ? &chain[i + 1] : nullptr, nullptr};
    }
    CHECK(tree_depth(chain.data()) == n);
    CHECK(count_nodes(chain.data()) == n);
}
