#pragma once
// A parent owns its children. Each child points back at its parent.

#include <memory>
#include <string>
#include <vector>

struct Person {
    std::string name;
    std::shared_ptr<Person> parent;  // TODO: this makes a cycle
    std::vector<std::shared_ptr<Person>> children;
};

// Makes child one of parent's children, and parent child's parent.
void adopt(const std::shared_ptr<Person>& parent, const std::shared_ptr<Person>& child);

// "Charles < Ada < Byron": the person's name, then each ancestor's.
std::string lineage(const Person& p);
