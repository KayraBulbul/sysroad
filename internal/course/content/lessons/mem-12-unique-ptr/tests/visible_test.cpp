#include "doctest.h"

#include <sstream>

#include "tree.hpp"

TEST_CASE("child finds or makes, in sorted order") {
    Entry root;
    Entry* b = child(root, "b");
    Entry* a = child(root, "a");
    REQUIRE(b != nullptr);
    CHECK(child(root, "b") == b);
    REQUIRE(root.children.size() == 2);
    CHECK(root.children[0].get() == a);
    CHECK(root.children[1].get() == b);
}

TEST_CASE("add_path") {
    Entry root;
    add_path(root, "src/util/str.cpp");
    add_path(root, "src/main.cpp");
    REQUIRE(root.children.size() == 1);
    Entry& src = *root.children[0];
    CHECK(src.name == "src");
    REQUIRE(src.children.size() == 2);
    CHECK(src.children[0]->name == "main.cpp");
    CHECK(src.children[1]->name == "util");
}

TEST_CASE("print_tree") {
    Entry root;
    root.name = ".";
    add_path(root, "src/main.cpp");
    add_path(root, "src/util/str.cpp");
    add_path(root, "README.md");
    std::ostringstream out;
    print_tree(root, out);
    CHECK(out.str() == ".\n"
                       "├── README.md\n"
                       "└── src\n"
                       "    ├── main.cpp\n"
                       "    └── util\n"
                       "        └── str.cpp\n"
                       "\n"
                       "3 directories, 3 files\n");
}
