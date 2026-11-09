#include "doctest.h"

#include <sstream>
#include <string>

#include "tree.hpp"

TEST_CASE("pointers stay valid as siblings are added") {
    Entry root;
    Entry* m = child(root, "m");
    m->name = "m";
    for (int i = 0; i < 200; ++i) {
        child(root, std::to_string(i));
    }
    CHECK(m->name == "m");  // ASan would catch a dangling pointer here
    CHECK(child(root, "m") == m);
    CHECK(root.children.size() == 201);
    for (std::size_t i = 1; i < root.children.size(); ++i) {
        CHECK(root.children[i - 1]->name < root.children[i]->name);
    }
}

TEST_CASE("add_path skips empty parts and dots") {
    Entry root;
    add_path(root, "./a//b/");
    add_path(root, "a/./c");
    add_path(root, "");
    add_path(root, "/");
    REQUIRE(root.children.size() == 1);
    Entry& a = *root.children[0];
    CHECK(a.name == "a");
    REQUIRE(a.children.size() == 2);
    CHECK(a.children[0]->name == "b");
    CHECK(a.children[1]->name == "c");
}

TEST_CASE("detach hands over a subtree") {
    Entry root;
    add_path(root, "keep/x");
    add_path(root, "take/y/z");
    Entry* y = root.children[1]->children[0].get();
    std::unique_ptr<Entry> t = detach(root, "take");
    REQUIRE(t != nullptr);
    CHECK(t->name == "take");
    CHECK(t->children[0].get() == y);  // moved, not copied
    REQUIRE(root.children.size() == 1);
    CHECK(root.children[0]->name == "keep");
    CHECK(detach(root, "take") == nullptr);
    CHECK(detach(root, "nope") == nullptr);
    CHECK(detach(root, "kee") == nullptr);
}

TEST_CASE("print_tree: singular counts and │ prefixes") {
    Entry root;
    root.name = "top";
    add_path(root, "a/one");
    add_path(root, "a/two/deep");
    add_path(root, "b");
    std::ostringstream out;
    print_tree(root, out);
    CHECK(out.str() == "top\n"
                       "├── a\n"
                       "│   ├── one\n"
                       "│   └── two\n"
                       "│       └── deep\n"
                       "└── b\n"
                       "\n"
                       "3 directories, 3 files\n");

    Entry single;
    single.name = ".";
    add_path(single, "d/f");
    std::ostringstream o2;
    print_tree(single, o2);
    CHECK(o2.str() == ".\n└── d\n    └── f\n\n2 directories, 1 file\n");

    Entry empty;
    empty.name = ".";
    std::ostringstream o3;
    print_tree(empty, o3);
    CHECK(o3.str() == ".\n\n1 directory, 0 files\n");
}

TEST_CASE("names sort by bytes") {
    Entry root;
    for (const char* n : {"b", "B", "a", "_x", "10", "9"}) {
        child(root, n);
    }
    std::string order;
    for (auto& c : root.children) {
        order += c->name + " ";
    }
    CHECK(order == "10 9 B _x a b ");
}
