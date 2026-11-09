#include "list.hpp"

#include <utility>

IntList::~IntList() {
    // TODO: free the nodes in a loop. As it is, head_'s destructor
    // recurses down the whole list.
}

void IntList::push_front(int v) {
    // TODO
    (void)v;
}

void IntList::push_back(int v) {
    // TODO
    (void)v;
}

int IntList::pop_front() {
    // TODO
    return 0;
}

bool IntList::contains(int v) const {
    // TODO
    (void)v;
    return false;
}

std::vector<int> IntList::to_vector() const {
    // TODO
    return {};
}

void IntList::reverse() {
    // TODO
}
