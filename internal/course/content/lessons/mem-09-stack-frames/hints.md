A list needs no stack at all, just a pointer that walks:

```cpp
long total = 0;
for (const Node* n = head; n != nullptr; n = n->next)
    total += n->value;
```
---
For the depth, keep each node with its depth:

```cpp
std::vector<std::pair<const Tree*, int>> todo;
if (root) todo.push_back({root, 1});
int deepest = 0;
while (!todo.empty()) {
    auto [t, d] = todo.back();
    todo.pop_back();
    deepest = std::max(deepest, d);
    if (t->left) todo.push_back({t->left, d + 1});
    if (t->right) todo.push_back({t->right, d + 1});
}
```

`auto [t, d] = ...` is a *structured binding*: it unpacks the pair into
two names.
---
`count_nodes` is the same loop with a plain `std::vector<const Tree*>`
and a counter instead of the depth.
