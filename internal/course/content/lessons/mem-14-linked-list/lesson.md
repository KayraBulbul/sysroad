---
title: A linked list with unique_ptr
minutes: 25
files: [list.cpp]
run: {}
deeper:
  - title: "Learning Rust With Entirely Too Many Linked Lists (the same ideas, in Rust)"
    url: https://rust-unofficial.github.io/too-many-lists/
---
A singly linked list is the classic ownership exercise. Each node owns
the next one, so the list is a chain of `unique_ptr`s:

```cpp
struct Node {
    int value;
    std::unique_ptr<Node> next;    // owns the rest of the list
};
std::unique_ptr<Node> head_;        // owns the first node
Node* tail_ = nullptr;              // looks at the last one, owns nothing
```

`tail_` is a plain pointer because the last node already has an owner,
the node before it. Having two owners is exactly what `unique_ptr`
rules out. The raw pointer is just a shortcut, so `push_back` doesn't
have to walk the whole list.

Moving pointers around is where it gets interesting. To drop the first
node:

```cpp
head_ = std::move(head_->next);
```

The move takes the second node out of the first. Then the assignment
deletes the old first node, whose `next` is now empty, so nothing else
goes with it.

Then there's the destructor. The default one destroys `head_`, which
destroys its `next`, which destroys *its* `next`, and so on: one level
of recursion per node. A million nodes overflow the stack, just like in
lesson 2.2.1. Your destructor has to free the nodes in a loop.

## Assignment

- [ ] `push_front`, `push_back` and `pop_front`, keeping `tail_` and `size_` right
- [ ] `contains` and `to_vector` walk the list
- [ ] `reverse` flips the links in place, without allocating
- [ ] The destructor frees a million nodes without recursing
