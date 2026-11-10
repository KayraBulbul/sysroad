#pragma once
// The six functions in the listings. You won't need to change this file.

#include "item.hpp"

// One node of a linked list. The last node's next is nullptr.
struct Node {
    long key;
    long value;
    const Node* next;
};

extern "C" {
long kilo(const long* a, long n);
int lima(const int* a, long i);
void mike(long* a, const long* b);

// The value of the stock: price times count, added up over n items.
long november(const Item* items, long n);

// The item's id, shelf and tag, added together.
int oscar(const Item* item);

long papa(const Node* node, long key);
}
