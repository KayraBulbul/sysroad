`push_front` makes a node that owns the old head:

```cpp
head_ = std::make_unique<Node>(Node{v, std::move(head_)});
if (!tail_) tail_ = head_.get();     // it was empty
++size_;
```
---
`push_back` keeps a raw pointer before it gives the node away:

```cpp
auto n = std::make_unique<Node>(Node{v, nullptr});
Node* raw = n.get();
if (tail_) tail_->next = std::move(n);
else head_ = std::move(n);
tail_ = raw;
++size_;
```

In `pop_front`, set `tail_ = nullptr` when the list becomes empty.
---
The destructor, one node at a time:
`while (head_) head_ = std::move(head_->next);`.
---
`reverse`: take each node off the front and put it on the front of a
new chain.

```cpp
std::unique_ptr<Node> done;
tail_ = head_.get();                       // the old first is the new last
while (head_) {
    std::unique_ptr<Node> rest = std::move(head_->next);
    head_->next = std::move(done);
    done = std::move(head_);
    head_ = std::move(rest);
}
head_ = std::move(done);
```
