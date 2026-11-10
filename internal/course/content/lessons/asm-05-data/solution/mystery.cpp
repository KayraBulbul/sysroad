#include "mystery.hpp"

long kilo(const long* a, long n) {
    long sum = 0;
    for (long i = 0; i < n; ++i) {
        sum += a[i];
    }
    return sum;
}

int lima(const int* a, long i) {
    return a[i] + a[i + 2];
}

void mike(long* a, const long* b) {
    *a += *b;
}

long november(const Item* items, long n) {
    long total = 0;
    for (long i = 0; i < n; ++i) {
        total += items[i].price * items[i].count;
    }
    return total;
}

int oscar(const Item* item) {
    return item->id + item->shelf + item->tag;
}

long papa(const Node* node, long key) {
    while (node != nullptr) {
        if (node->key == key) {
            return node->value;
        }
        node = node->next;
    }
    return -1;
}
